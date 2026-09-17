#include "core/Pipeline.h"
#include "core/Window.h"
#include "core/Timer.h"
#include "utils/Student.h"
#include "utils/image.h"

#include <array>
#include <cstdlib>
#include <iomanip>
#include <iostream>

struct Scene {
  float         scale;
  std::uint32_t columns;
  std::uint32_t vertexLoop;
  std::uint32_t fragmentLoop;
};

struct SceneDesc {
  const char*   name;
  float         scale;
  std::uint32_t instances;
  std::uint32_t columns;
};

static const std::array<SceneDesc, 3> SCENES = {{
  { "1 triangle, small    ", 0.04f, 1,      1   },
  { "1 triangle, fullscreen", 4.0f, 1,      1   },
  { "100k instances, small ", 0.04f, 100000, 317 },
}};

static const std::array<std::uint32_t, 3> LOOPS = { 0, 500, 2000 };

int main() {
  try {
    VkVertexInputBindingDescription binding = Vertex::bindingDescription();
    std::vector<VkVertexInputAttributeDescription> attrs = Vertex::attributeDescriptions();

    std::uint32_t vertexCount = static_cast<std::uint32_t>(TRIANGLE.size());
    bool ready = vertexCount > 0 && binding.stride > 0 && attrs.size() == 2;

    GLFWwindow*  handle     = createHiddenWindow("Lab 06 - Part II", WIDTH, HEIGHT);
    Device       dev        = createDevice(handle);
    VkRenderPass renderPass = createRenderPass(dev);
    Target       target     = createTarget(dev, renderPass, WIDTH, HEIGHT);
    Window       win        = createSwapchain(dev, handle);

    Buffer vertices = createBuffer(dev, sizeof(Vertex) * std::max(vertexCount, 1u),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (vertexCount > 0)
      uploadBuffer(vertices, TRIANGLE.data(), sizeof(Vertex) * vertexCount);

    std::vector<std::uint8_t> params = uniformBlock(0, 0);
    Buffer uniform = createBuffer(dev, params.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uploadBuffer(uniform, params.data(), params.size());

    Descriptors      descs  = createDescriptors(dev, uniform);
    VkPipelineLayout layout = createPipelineLayout(dev, descs, sizeof(Scene));

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

    if (!ready) {
      std::cout << "  incomplete     nothing was measured: TASK 1 is not finished\n";
      timePass(dev, 0, 1, [&](VkCommandBuffer cmd) {
          vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
          vkCmdEndRenderPass(cmd);
      });
    } else {
      VkShaderModule vertShader = createShaderModule(dev, "cost.vert");
      VkShaderModule fragShader = createShaderModule(dev, "cost.frag");
      VkPipeline     pipeline   =
        createPipeline(dev, renderPass, layout, vertShader, fragShader, target);

      std::cout << "  loops          " << LOOPS[0] << ", " << LOOPS[1] << ", " << LOOPS[2]
                << " (ms median, in the vertex stage then the fragment stage)\n";

      for (const SceneDesc& s : SCENES) {
        std::cout << "  " << s.name << " ";
        for (std::uint32_t stage = 0; stage < 2; stage++) {
          for (std::uint32_t loops : LOOPS) {
            params = uniformBlock(loops, 0);
            uploadBuffer(uniform, params.data(), params.size());

            Scene push{};
            push.scale        = s.scale;
            push.columns      = s.columns;
            push.vertexLoop   = stage == 0 ? 1u : 0u;
            push.fragmentLoop = stage == 1 ? 1u : 0u;

            Timing t = timePass(dev, WARMUP_FRAMES, MEASURED_FRAMES, [&](VkCommandBuffer cmd) {
                vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
                VkDeviceSize offset = 0;
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                                        &descs.set, 0, nullptr);
                vkCmdBindVertexBuffers(cmd, 0, 1, &vertices.handle, &offset);
                vkCmdPushConstants(cmd, layout,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                   0, sizeof(Scene), &push);
                vkCmdDraw(cmd, vertexCount, s.instances, 0, 0);
                vkCmdEndRenderPass(cmd);
            });
            std::cout << std::setw(8) << t.medianMs;
          }
          std::cout << (stage == 0 ? "  |" : "");
        }
        std::cout << "\n";
      }

      vkDestroyPipeline(dev.device, pipeline, nullptr);
      vkDestroyShaderModule(dev.device, fragShader, nullptr);
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
