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
  if (binding.stride == 0 || attrs.size() != 2)
    std::cout << "                 TASK 1: finish Part I first, the square reuses it\n";
  if (QUAD.size() != 4)
    std::cout << "                 TASK 4a: QUAD has " << QUAD.size() << " of 4 corners\n";
  if (QUAD_INDICES.size() != 6)
    std::cout << "                 TASK 4b: QUAD_INDICES has " << QUAD_INDICES.size()
              << " of 6 indices\n";
}

static void reportBytes() {
  std::size_t indices  = QUAD_INDICES.size();
  std::size_t unindexed = indices * sizeof(Vertex);
  std::size_t indexed   = QUAD.size() * sizeof(Vertex) + indices * sizeof(std::uint16_t);

  std::cout << "  unindexed      " << indices << " vertices x " << sizeof(Vertex) << " B = "
            << unindexed << " bytes\n";
  std::cout << "  indexed        " << QUAD.size() << " x " << sizeof(Vertex) << " B + " << indices
            << " x 2 B = " << indexed << " bytes\n";
  if (unindexed > 0)
    std::cout << "  saving         "
              << (1.0 - static_cast<double>(indexed) / static_cast<double>(unindexed)) * 100.0
              << "%\n";
}

int main() {
  try {
    VkVertexInputBindingDescription binding = Vertex::bindingDescription();
    std::vector<VkVertexInputAttributeDescription> attrs = Vertex::attributeDescriptions();
    std::vector<std::uint8_t> params = uniformBlock(0, 0);

    std::uint32_t indexCount = static_cast<std::uint32_t>(QUAD_INDICES.size());
    bool ready = binding.stride > 0 && attrs.size() == 2 && QUAD.size() == 4 && indexCount == 6;

    GLFWwindow*  handle     = createHiddenWindow("Lab 06 - Part V", WIDTH, HEIGHT);
    Device       dev        = createDevice(handle);
    VkRenderPass renderPass = createRenderPass(dev);
    Target       target     = createTarget(dev, renderPass, WIDTH, HEIGHT);
    Window       win        = createSwapchain(dev, handle);

    Buffer uniform = createBuffer(dev, params.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uploadBuffer(uniform, params.data(), params.size());

    Buffer vertices = createBuffer(dev, sizeof(Vertex) * std::max<std::size_t>(QUAD.size(), 1),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (!QUAD.empty())
      uploadBuffer(vertices, QUAD.data(), sizeof(Vertex) * QUAD.size());

    Buffer indices = createBuffer(dev, sizeof(std::uint16_t) * std::max(indexCount, 1u),
                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    if (indexCount > 0)
      uploadBuffer(indices, QUAD_INDICES.data(), sizeof(std::uint16_t) * indexCount);

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
            vkCmdBindIndexBuffer(cmd, indices.handle, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
        }
        vkCmdEndRenderPass(cmd);
    });

    std::vector<std::uint8_t> rgba = readTarget(dev, target);
    double drawn = coverage(rgba);
    double diff  = diffFromReference("index", rgba, target.width, target.height);

    std::cout << std::fixed << std::setprecision(1);
    reportBytes();
    std::cout << "  index type     VK_INDEX_TYPE_UINT16\n";
    std::cout << "  rasterized     " << drawn * 100.0 << "% of the target\n";

    if (diff < 0.0 && ready && drawn > 0.0) {
      writePng(std::string(REFERENCE_DIR) + "/index.png", rgba, target.width, target.height);
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
    } else if (drawn > 0.0 && drawn < 0.24) {
      std::cout << "  hint           only part of the square was drawn. One of your two\n"
                << "                 triangles is wound the other way, so it was culled\n";
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
    destroyBuffer(dev, indices);
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
