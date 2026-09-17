#include "utils/image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <cstdlib>

void writePng(const std::string& path,
              const std::vector<std::uint8_t>& rgba,
              std::uint32_t width,
              std::uint32_t height)
{
  stbi_write_png(path.c_str(), static_cast<int>(width), static_cast<int>(height), 4, rgba.data(),
                 static_cast<int>(width) * 4);
}

double coverage(const std::vector<std::uint8_t>& rgba) {
  std::size_t drawn = 0;
  for (std::size_t i = 0; i < rgba.size(); i += 4)
    if (rgba[i] != 0 || rgba[i + 1] != 0 || rgba[i + 2] != 0) ++drawn;
  return static_cast<double>(drawn) / static_cast<double>(rgba.size() / 4);
}

double diffFromReference(const std::string& name,
                         const std::vector<std::uint8_t>& rgba,
                         std::uint32_t width,
                         std::uint32_t height)
{
  std::string path = std::string(REFERENCE_DIR) + "/" + name + ".png";

  int refWidth  = 0;
  int refHeight = 0;
  int channels  = 0;
  std::uint8_t* ref = stbi_load(path.c_str(), &refWidth, &refHeight, &channels, 4);
  if (ref == nullptr) return -1.0;

  if (static_cast<std::uint32_t>(refWidth) != width ||
      static_cast<std::uint32_t>(refHeight) != height) {
    stbi_image_free(ref);
    return 100.0;
  }

  std::size_t different = 0;
  for (std::size_t i = 0; i < rgba.size(); i += 4)
    for (std::size_t c = 0; c < 3; ++c)
      if (std::abs(static_cast<int>(rgba[i + c]) - static_cast<int>(ref[i + c])) > 2) {
        ++different;
        break;
      }

  stbi_image_free(ref);
  return 100.0 * static_cast<double>(different) / static_cast<double>(rgba.size() / 4);
}

std::vector<std::uint8_t> pixelAt(const std::vector<std::uint8_t>& rgba,
                                  std::uint32_t width,
                                  std::uint32_t x,
                                  std::uint32_t y)
{
  std::size_t i = (static_cast<std::size_t>(y) * width + x) * 4;
  return { rgba[i], rgba[i + 1], rgba[i + 2] };
}
