#include "core/Pipeline.h"
#include "core/Window.h"
#include "core/Timer.h"
#include "utils/Student.h"
#include "utils/image.h"

#include <array>
#include <cstdlib>
#include <iomanip>
#include <iostream>

struct Layer {
  float depth;
  float scale;
};

static constexpr std::uint32_t LAYERS   = 8;
static constexpr std::uint32_t COST     = 2000;
static constexpr float         COVERAGE = 4.0f;

static const std::array<Variant, 4> VARIANTS = {
  Variant::EarlyZFrontToBack,
  Variant::EarlyZBackToFront,
  Variant::WriteDepthFrontToBack,
  Variant::WriteDepthBackToFront,
};

static const char* variantName(Variant v) {
  switch (v) {
    case Variant::EarlyZFrontToBack:     return "early-Z      front-to-back";
    case Variant::EarlyZBackToFront:     return "early-Z      back-to-front";
    case Variant::WriteDepthFrontToBack: return "writes depth front-to-back";
    default:                             return "writes depth back-to-front";
  }
}

int main() {
  try {
    VkVertexInputBindingDescription binding = Vertex::bindingDescription();
    std::vector<VkVertexInputAttributeDescription> attrs = Vertex::attributeDescriptions();
    std::vector<std::uint8_t> params = uniformBlock(COST, 0);

    std::uint32_t vertexCount = static_cast<std::uint32_t>(TRIANGLE.size());
    bool taskOne   = vertexCount > 0 && binding.stride > 0 && attrs.size() == 2;
    bool taskThree = !pipelineState(Variant::EarlyZFrontToBack).fragShader.empty();

    GLFWwindow*  handle     = createHiddenWindow("Lab 06 - Part IV", WIDTH, HEIGHT);
    Device       dev        = createDevice(handle);
    VkRenderPass renderPass = createRenderPass(dev, true);
    Target       target     = createTarget(dev, renderPass, WIDTH, HEIGHT, true);
    Window       win        = createSwapchain(dev, handle);

    Buffer uniform = createBuffer(dev, params.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uploadBuffer(uniform, params.data(), params.size());

    Buffer vertices = createBuffer(dev, sizeof(Vertex) * std::max(vertexCount, 1u),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (vertexCount > 0)
      uploadBuffer(vertices, TRIANGLE.data(), sizeof(Vertex) * vertexCount);

    Descriptors      descs  = createDescriptors(dev, uniform);
    VkPipelineLayout layout = createPipelineLayout(dev, descs, sizeof(Layer));

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  layers         " << LAYERS << " copies, " << COST << " loops each\n";

    VkClearValue clears[2]{};
    clears[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clears[1].depthStencil = { 0.0f, 0 };

    VkRenderPassBeginInfo passInfo{};
    passInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    passInfo.renderPass        = renderPass;
    passInfo.framebuffer       = target.framebuffer;
    passInfo.renderArea.extent = { target.width, target.height };
    passInfo.clearValueCount   = 2;
    passInfo.pClearValues      = clears;

    if (!taskOne || !taskThree) {
      std::cout << "  incomplete     nothing was measured because student.cpp is not finished:\n";
      if (!taskOne)
        std::cout << "                 TASK 1: finish Part I first, this part reuses it\n";
      if (!taskThree)
        std::cout << "                 TASK 3e: pipelineState() left fragShader empty\n";

      timePass(dev, 0, 1, [&](VkCommandBuffer cmd) {
          vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
          vkCmdEndRenderPass(cmd);
      });
    } else {
      VkShaderModule vertShader = createShaderModule(dev, "earlyz.vert");

      for (Variant v : VARIANTS) {
        PipelineState state = pipelineState(v);

        VkShaderModule fragShader = createShaderModule(dev, state.fragShader);
        VkPipeline     pipeline   =
          createPipeline(dev, renderPass, layout, vertShader, fragShader, target, state);

        Timing timing = timePass(dev, WARMUP_FRAMES, MEASURED_FRAMES, [&](VkCommandBuffer cmd) {
            vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
            VkDeviceSize offset = 0;
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                                    &descs.set, 0, nullptr);
            vkCmdBindVertexBuffers(cmd, 0, 1, &vertices.handle, &offset);

            for (std::uint32_t i = 0; i < LAYERS; i++) {
              std::uint32_t index = state.drawOrder == DrawOrder::FrontToBack ? LAYERS - 1 - i : i;
              Layer push{};
              push.depth = static_cast<float>(index + 1) / static_cast<float>(LAYERS + 1);
              push.scale = COVERAGE;
              vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Layer), &push);
              vkCmdDraw(cmd, vertexCount, 1, 0, 0);
            }
            vkCmdEndRenderPass(cmd);
        });

        std::cout << "  " << variantName(v) << "   " << timing.medianMs << " ms median, "
                  << timing.p95Ms << " ms p95\n";

        vkDestroyPipeline(dev.device, pipeline, nullptr);
        vkDestroyShaderModule(dev.device, fragShader, nullptr);
      }
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
