#include "core/Pipeline.h"
#include "core/Window.h"
#include "core/Timer.h"
#include "utils/Student.h"
#include "utils/image.h"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>

static void reportIncomplete(const VkVertexInputBindingDescription& binding,
                             const std::vector<VkVertexInputAttributeDescription>& attrs)
{
  std::cout << "  incomplete     nothing was drawn because student.cpp is not finished:\n";
  if (TRIANGLE.empty())
    std::cout << "                 TASK 1a: TRIANGLE has no vertices\n";
  if (binding.stride == 0)
    std::cout << "                 TASK 1b: stride is still 0\n";
  if (attrs.size() != 2)
    std::cout << "                 TASK 1c: " << attrs.size() << " of 2 attributes described\n";
}

static void reportCentroid(const std::vector<std::uint8_t>& rgba, const Target& target) {
  glm::vec2 centroid(0.0f);
  for (const Vertex& v : TRIANGLE)
    centroid += v.pos;
  centroid /= static_cast<float>(TRIANGLE.size());

  std::uint32_t x = static_cast<std::uint32_t>((centroid.x * 0.5f + 0.5f) * target.width);
  std::uint32_t y = static_cast<std::uint32_t>((centroid.y * 0.5f + 0.5f) * target.height);
  std::vector<std::uint8_t> pixel = pixelAt(rgba, target.width, x, y);

  std::cout << "  centroid       pixel (" << x << ", " << y << ") reads ("
            << static_cast<int>(pixel[0]) << ", " << static_cast<int>(pixel[1]) << ", "
            << static_cast<int>(pixel[2]) << ")\n";
}

int main() {
  try {
    VkVertexInputBindingDescription binding = Vertex::bindingDescription();
    std::vector<VkVertexInputAttributeDescription> attrs = Vertex::attributeDescriptions();
    std::vector<std::uint8_t> params = uniformBlock(0, 0);

    std::uint32_t vertexCount = static_cast<std::uint32_t>(TRIANGLE.size());
    bool ready = vertexCount > 0 && binding.stride > 0 && attrs.size() == 2;

    GLFWwindow*  handle     = createHiddenWindow("Lab 06 - Part I", WIDTH, HEIGHT);
    Device       dev        = createDevice(handle);
    VkRenderPass renderPass = createRenderPass(dev);
    Target       target     = createTarget(dev, renderPass, WIDTH, HEIGHT);
    Window       win        = createSwapchain(dev, handle);

    Buffer uniform = createBuffer(dev, params.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uploadBuffer(uniform, params.data(), params.size());

    Buffer vertices = createBuffer(dev, sizeof(Vertex) * std::max(vertexCount, 1u),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (vertexCount > 0)
      uploadBuffer(vertices, TRIANGLE.data(), sizeof(Vertex) * vertexCount);

    Descriptors      descs  = createDescriptors(dev, uniform);
    VkPipelineLayout layout = createPipelineLayout(dev, descs);

    VkShaderModule vertShader = VK_NULL_HANDLE;
    VkShaderModule fragShader = VK_NULL_HANDLE;
    VkPipeline     pipeline   = VK_NULL_HANDLE;
    if (ready) {
      vertShader = createShaderModule(dev, "triangle.vert");
      fragShader = createShaderModule(dev, "triangle.frag");
      pipeline   = createPipeline(dev, renderPass, layout, vertShader, fragShader, target);
    }

    Timing timing = timePass(dev, WARMUP_FRAMES, MEASURED_FRAMES, [&](VkCommandBuffer cmd) {
        VkClearValue clear{};
        clear.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        VkRenderPassBeginInfo passInfo{};
        passInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        passInfo.renderPass        = renderPass;
        passInfo.framebuffer       = target.framebuffer;
        passInfo.renderArea.extent = { target.width, target.height };
        passInfo.clearValueCount   = 1;
        passInfo.pClearValues      = &clear;

        vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
        if (pipeline != VK_NULL_HANDLE) {
            VkDeviceSize offset = 0;
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                                    &descs.set, 0, nullptr);
            vkCmdBindVertexBuffers(cmd, 0, 1, &vertices.handle, &offset);
            vkCmdDraw(cmd, vertexCount, 1, 0, 0);
        }
        vkCmdEndRenderPass(cmd);
    });

    std::vector<std::uint8_t> rgba = readTarget(dev, target);
    double drawn = coverage(rgba);
    double diff  = diffFromReference("triangle", rgba, target.width, target.height);

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  vertex buffer  " << vertexCount << " vertices, stride " << binding.stride
              << ", offsets [";
    for (const VkVertexInputAttributeDescription& attr : attrs)
      std::cout << attr.offset << " ";
    std::cout << "]\n";
    std::cout << "  uniform block  " << params.size() << " bytes\n";
    std::cout << "  rasterized     " << drawn * 100.0 << "% of the target\n";

    if (vertexCount > 0) reportCentroid(rgba, target);

    if (diff < 0.0 && ready && drawn > 0.0) {
      writePng(std::string(REFERENCE_DIR) + "/triangle.png", rgba, target.width, target.height);
      std::cout << "  image          no reference found, wrote one: check it by eye\n";
    }
    else if (diff < 0.0) {
      std::cout << "  image          not checked\n";
    }
    else if (diff < 0.5) {
      std::cout << "  image          MATCH\n";
    } else {
      std::cout << "  image          DIFF " << diff << "%\n";
    }

    if (!ready) {
      reportIncomplete(binding, attrs);
    } else if (std::all_of(params.begin(), params.begin() + 64,
                           [](std::uint8_t b) { return b == 0; })) {
      std::cout << "  hint           your uniform block is all zeros, so every vertex\n"
                << "                 collapses to the origin. TASK 2 writes that matrix\n";
    } else if (drawn == 0.0) {
      std::cout << "  hint           nothing was rasterized. Check your winding against\n"
                << "                 VK_FRONT_FACE_COUNTER_CLOCKWISE, and the sign of y\n";
    }

    std::cout << std::setprecision(3);
    std::cout << "  gpu time       " << timing.medianMs << " ms median, " << timing.p95Ms
              << " ms p95" << std::endl;

    showWindow(win);
    while (windowOpen(win))
      presentTarget(dev, win, target);

    if (pipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(dev.device, pipeline, nullptr);
      vkDestroyShaderModule(dev.device, fragShader, nullptr);
      vkDestroyShaderModule(dev.device, vertShader, nullptr);
    }
    vkDestroyPipelineLayout(dev.device, layout, nullptr);
    destroyDescriptors(dev, descs);
    destroyBuffer(dev, vertices);
    destroyBuffer(dev, uniform);
    destroySwapchain(dev, win);
    destroyTarget(dev, target);
    destroyRenderPass(dev, renderPass);
    destroyDevice(dev);
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
