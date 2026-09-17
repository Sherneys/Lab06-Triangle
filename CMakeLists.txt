cmake_minimum_required(VERSION 3.24)
project(lab06 LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    "in-source builds overwrite this project's Makefile. Use: cmake -S . -B build")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Vulkan
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# GLM ships with the Vulkan SDK, but not always on the default include path
find_path(GLM_INCLUDE_DIR glm/glm.hpp
  HINTS $ENV{VULKAN_SDK}/include ${Vulkan_INCLUDE_DIR}
  PATHS /usr/local/include /opt/homebrew/include /usr/include
)

if(NOT GLM_INCLUDE_DIR)
  message(FATAL_ERROR "glm/glm.hpp not found. It comes with the Vulkan SDK: set VULKAN_SDK "
                      "or source the SDK's setup-env.sh. On Linux with apt, install libglm-dev")
endif()

add_library(glm INTERFACE)
target_include_directories(glm SYSTEM INTERFACE ${GLM_INCLUDE_DIR})

# stb is two headers, fetched once, used only by src/utils/image.cpp
set(STB_COMMIT f0569113c93ad095470c54bf34a17b36646bbbb5)
set(STB_DIR ${CMAKE_SOURCE_DIR}/external/stb)

foreach(HEADER stb_image.h stb_image_write.h)
  if(NOT EXISTS ${STB_DIR}/${HEADER})
    message(STATUS "fetching ${HEADER}")
    file(DOWNLOAD
      https://raw.githubusercontent.com/nothings/stb/${STB_COMMIT}/${HEADER}
      ${STB_DIR}/${HEADER}
      STATUS STB_STATUS
      TLS_VERIFY ON
    )
    list(GET STB_STATUS 0 STB_CODE)
    if(NOT STB_CODE EQUAL 0)
      file(REMOVE ${STB_DIR}/${HEADER})
      message(FATAL_ERROR "could not download ${HEADER}. Run external/fetch.sh, "
                          "or copy the two stb headers into external/stb by hand")
    endif()
  endif()
endforeach()

file(SHA256 ${STB_DIR}/stb_image.h STB_IMAGE_HASH)
if(NOT STB_IMAGE_HASH STREQUAL
   "594c2fe35d49488b4382dbfaec8f98366defca819d916ac95becf3e75f4200b3")
  message(FATAL_ERROR "external/stb/stb_image.h does not match the pinned commit")
endif()

add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE ${STB_DIR})

# ── Core library ──────────────────────────────────────────────────────────────
add_library(lab06core STATIC
  src/core/Device.cpp
  src/core/Buffer.cpp
  src/core/Target.cpp
  src/core/Pipeline.cpp
  src/core/Timer.cpp
  src/core/Window.cpp
  src/utils/utils.cpp
  src/utils/image.cpp
)

target_include_directories(lab06core PUBLIC
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(lab06core PUBLIC
  Vulkan::Vulkan
  glfw
  glm
  stb
)

target_compile_options(lab06core PUBLIC -Wall -Wextra)

target_compile_definitions(lab06core PUBLIC
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  GLM_FORCE_RADIANS
  SHADER_DIR="${CMAKE_BINARY_DIR}/shaders"
  REFERENCE_DIR="${CMAKE_SOURCE_DIR}/reference"
)

# ── One executable per part ───────────────────────────────────────────────────
set(STUDENT "${CMAKE_SOURCE_DIR}/student.cpp" CACHE FILEPATH "which student.cpp to build")
set(PARTS info triangle cost stripes earlyz index probe)

foreach(PART ${PARTS})
 file(GLOB PART_SOURCE src/${PART}*.cpp)
 add_executable(${PART} ${PART_SOURCE} ${STUDENT})
 target_link_libraries(${PART} PRIVATE lab06core)
endforeach()

# ── Shader compilation ────────────────────────────────────────────────────────
# glslc ships with the Vulkan SDK; glslangValidator is the fallback name.
find_program(GLSLC NAMES glslc glslangValidator)

if(NOT GLSLC)
  message(FATAL_ERROR "no GLSL compiler found. glslc comes with the Vulkan SDK; "
                      "on Linux, apt install glslang-tools")
endif()

file(GLOB GLSL_FILES
  "${CMAKE_SOURCE_DIR}/shaders/*.vert"
  "${CMAKE_SOURCE_DIR}/shaders/*.frag"
  "${CMAKE_SOURCE_DIR}/shaders/*.comp"
)

set(SPIRV_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

if(GLSLC MATCHES "glslangValidator")
  set(GLSLC_ARGS -V)
endif()

foreach(GLSL ${GLSL_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${SPIRV_OUTPUT_DIR}/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSLC} ${GLSLC_ARGS} ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL}
  )
  list(APPEND SPIRV_FILES ${SPIRV})
endforeach()

add_custom_target(shaders DEPENDS ${SPIRV_FILES})
add_dependencies(lab06core shaders)
