#include "core/Pipeline.h"
#include "core/Window.h"
#include "core/Timer.h"
#include "utils/Student.h"
#include "utils/image.h"

#include <array>
#include <cstdlib>
#include <iomanip>
#include <iostream>

struct Push {
  float         scale;
  std::uint32_t fetches;
  std::uint32_t useInstances;
};

struct Config {
  const char*   name;
  float         scale;
  std::uint32_t instances;
  std::uint32_t fetches;
  std::uint32_t loops;
};

static const std::array<Config, 3> SCENES = {{
  { "scene 1", 2.0f, 1,         1,  3000 },
  { "scene 2", 2.0f, 1,         64, 0    },
  { "scene 3", 0.6f, INSTANCES, 1,  0    },
}};

int main() {
  try {
    VkVertexInputBindingDescription binding = Vertex::bindingDescription();
    std::vector<VkVertexInputAttributeDescription> attrs = Vertex::attributeDescriptions();
    std::vector<glm::mat4> transforms = instanceBuffer();

    std::uint32_t indexCount = static_cast<std::uint32_t>(QUAD_INDICES.size());
    bool taskOne  = binding.stride > 0 && attrs.size() == 2;
    bool taskFour = QUAD.size() == 4 && indexCount == 6;
    bool taskFive = transforms.size() == INSTANCES;

    GLFWwindow*  handle     = createHiddenWindow("Lab 06 - Part VI", WIDTH, HEIGHT);
    Device       dev        = createDevice(handle);
    VkRenderPass renderPass = createRenderPass(dev);
    Target       target     = createTarget(dev, renderPass, WIDTH, HEIGHT);
    Window       win        = createSwapchain(dev, handle);

    Buffer vertices = createBuffer(dev, sizeof(Vertex) * std::max<std::size_t>(QUAD.size(), 1),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (!QUAD.empty())
      uploadBuffer(vertices, QUAD.data(), sizeof(Vertex) * QUAD.size());

    Buffer indices = createBuffer(dev, sizeof(std::uint16_t) * std::max(indexCount, 1u),
                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    if (indexCount > 0)
      uploadBuffer(indices, QUAD_INDICES.data(), sizeof(std::uint16_t) * indexCount);

    VkDeviceSize storageBytes = sizeof(glm::mat4) * INSTANCES;
    Buffer storage = createBuffer(dev, storageBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    if (taskFive)
      uploadBuffer(storage, transforms.data(), sizeof(glm::mat4) * transforms.size());

    std::vector<std::uint8_t> params = uniformBlock(0, 0);
    Buffer uniform = createBuffer(dev, params.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uploadBuffer(uniform, params.data(), params.size());

    Descriptors      descs  = createDescriptors(dev, uniform, storage);
    VkPipelineLayout layout = createPipelineLayout(dev, descs, sizeof(Push));

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
    std::cout << "  storage        " << storageBytes << " bytes, stride " << sizeof(glm::mat4) << "\n";

    if (!taskOne || !taskFour || !taskFive) {
      std::cout << "  incomplete     nothing was measured because student.cpp is not finished:\n";
      if (!taskOne)
        std::cout << "                 TASK 1: finish Part I first\n";
      if (!taskFour)
        std::cout << "                 TASK 4: finish Part V first, this part draws that square\n";
      if (!taskFive)
        std::cout << "                 TASK 5a: instanceBuffer() returned " << transforms.size()
                  << " of " << INSTANCES << " transforms\n";
      timePass(dev, 0, 1, [&](VkCommandBuffer cmd) {
          vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
          vkCmdEndRenderPass(cmd);
      });
    } else {
      VkShaderModule vertShader = createShaderModule(dev, "probe.vert");
      VkShaderModule fragShader = createShaderModule(dev, "probe.frag");
      VkPipeline     pipeline   =
        createPipeline(dev, renderPass, layout, vertShader, fragShader, target);

      enum class Draws { Single, Separate, Instanced };

      auto run = [&](const Config& c, float scale, std::uint32_t instances,
                     std::uint32_t fetches, Draws draws) {
        params = uniformBlock(c.loops, 0);
        uploadBuffer(uniform, params.data(), params.size());

        Push push{};
        push.scale        = scale;
        push.fetches      = fetches;
        push.useInstances = instances > 1 ? 1u : 0u;

        return timePass(dev, WARMUP_FRAMES, MEASURED_FRAMES, [&](VkCommandBuffer cmd) {
            vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
            VkDeviceSize offset = 0;
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                                    &descs.set, 0, nullptr);
            vkCmdBindVertexBuffers(cmd, 0, 1, &vertices.handle, &offset);
            vkCmdBindIndexBuffer(cmd, indices.handle, 0, VK_INDEX_TYPE_UINT16);
            vkCmdPushConstants(cmd, layout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0, sizeof(Push), &push);
            if (draws == Draws::Instanced)
              recordDraw(cmd, indexCount);
            else if (draws == Draws::Single)
              vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
            else
              for (std::uint32_t i = 0; i < instances; i++)
                vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, i);
            vkCmdEndRenderPass(cmd);
        });
      };

      std::cout << "                 baseline  half px  half inst  half bytes    cpu\n";
      for (const Config& c : SCENES) {
        Draws  how   = c.instances == 1 ? Draws::Single : Draws::Separate;
        Timing base  = run(c, c.scale, c.instances, c.fetches, how);
        Timing px    = run(c, c.scale * 0.7071f, c.instances, c.fetches, how);
        Timing inst  = run(c, c.scale, std::max(c.instances / 2, 1u), c.fetches, how);
        Timing bytes = run(c, c.scale, c.instances, std::max(c.fetches / 2, 1u), how);

        std::cout << "  " << c.name << std::setw(12) << base.medianMs << std::setw(9) << px.medianMs
                  << std::setw(11) << inst.medianMs << std::setw(12) << bytes.medianMs
                  << std::setw(8) << base.cpuMs << "\n";
      }

      const Config& many = SCENES[2];
      Timing before = run(many, many.scale, many.instances, many.fetches, Draws::Separate);
      Timing after  = run(many, many.scale, many.instances, many.fetches, Draws::Instanced);
      std::cout << "  " << many.instances << " draws    gpu " << before.medianMs << " ms, cpu "
                << before.cpuMs << " ms\n";
      std::cout << "  1 draw        gpu " << after.medianMs << " ms, cpu " << after.cpuMs << " ms\n";

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
    destroyBuffer(dev, storage);
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
