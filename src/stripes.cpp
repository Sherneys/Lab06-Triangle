#include "core/Pipeline.h"
#include "core/Window.h"
#include "core/Timer.h"
#include "utils/Student.h"
#include "utils/image.h"

#include <array>
#include <cstdlib>
#include <iomanip>
#include <iostream>

static constexpr std::uint32_t COST = 2000;

static const std::array<std::uint32_t, 8> WIDTHS = { 1, 2, 4, 8, 16, 32, 64, 128 };

int main() {
  try {
    VkVertexInputBindingDescription binding = Vertex::bindingDescription();
    std::vector<VkVertexInputAttributeDescription> attrs = Vertex::attributeDescriptions();

    std::uint32_t vertexCount = static_cast<std::uint32_t>(TRIANGLE.size());
    bool ready = vertexCount > 0 && binding.stride > 0 && attrs.size() == 2;

    GLFWwindow*  handle     = createHiddenWindow("Lab 06 - Part III", WIDTH, HEIGHT);
    Device       dev        = createDevice(handle);
    VkRenderPass renderPass = createRenderPass(dev);
    Target       target     = createTarget(dev, renderPass, WIDTH, HEIGHT);
    Window       win        = createSwapchain(dev, handle);

    Buffer vertices = createBuffer(dev, sizeof(Vertex) * std::max(vertexCount, 1u),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (vertexCount > 0)
      uploadBuffer(vertices, TRIANGLE.data(), sizeof(Vertex) * vertexCount);

    std::vector<std::uint8_t> params = uniformBlock(COST, 1);
    Buffer uniform = createBuffer(dev, params.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uploadBuffer(uniform, params.data(), params.size());

    Descriptors      descs  = createDescriptors(dev, uniform);
    VkPipelineLayout layout = createPipelineLayout(dev, descs);

    VkClearValue clear{};
    clear.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    VkRenderPassBeginInfo passInfo{};
    passInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    passInfo.renderPass        = renderPass;
    passInfo.framebuffer       = target.framebuffer;
    passInfo.renderArea.extent = { target.width, target.height };
    passInfo.clearValueCount   = 1;
    passInfo.pClearValues      = &clear;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  subgroup size  " << dev.subgroupSize << " lanes, " << COST << " loops\n";

    if (!ready) {
      std::cout << "  incomplete     nothing was measured: TASK 1 is not finished\n";
      timePass(dev, 0, 1, [&](VkCommandBuffer cmd) {
          vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
          vkCmdEndRenderPass(cmd);
      });
    } else {
      VkShaderModule vertShader = createShaderModule(dev, "stripes.vert");

      auto sweep = [&](const std::string& frag, std::uint32_t width) {
        VkShaderModule fragShader = createShaderModule(dev, frag);
        VkPipeline     pipeline   =
          createPipeline(dev, renderPass, layout, vertShader, fragShader, target);

        params = uniformBlock(COST, width);
        uploadBuffer(uniform, params.data(), params.size());

        Timing t = timePass(dev, WARMUP_FRAMES, MEASURED_FRAMES, [&](VkCommandBuffer cmd) {
            vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
            VkDeviceSize offset = 0;
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                                    &descs.set, 0, nullptr);
            vkCmdBindVertexBuffers(cmd, 0, 1, &vertices.handle, &offset);
            vkCmdDraw(cmd, vertexCount, 1, 0, 0);
            vkCmdEndRenderPass(cmd);
        });

        vkDestroyPipeline(dev.device, pipeline, nullptr);
        vkDestroyShaderModule(dev.device, fragShader, nullptr);
        return t.medianMs;
      };

      for (std::uint32_t width : WIDTHS)
        std::cout << "  stripe " << std::setw(4) << width << " px  " << std::setw(8)
                  << sweep("stripes.frag", width) << " ms median\n";

      std::cout << "  no branch      " << std::setw(8) << sweep("stripes_flat.frag", 1)
                << " ms median\n";

      vkDestroyShaderModule(dev.device, vertShader, nullptr);
    }

    std::vector<std::uint8_t> rgba = readTarget(dev, target);
    std::cout << std::setprecision(1);
    std::cout << "  rasterized     " << coverage(rgba) * 100.0 << "% of the target" << std::endl;

    showWindow(win);
    while (windowOpen(win))
      presentTarget(dev, win, target);

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
