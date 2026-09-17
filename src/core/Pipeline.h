#ifndef __PIPELINE_H_INCLUDED__
#define __PIPELINE_H_INCLUDED__

#include <string>

#include "core/Buffer.h"
#include "core/Target.h"

struct Descriptors {
  VkDescriptorSetLayout layout = VK_NULL_HANDLE;
  VkDescriptorPool pool = VK_NULL_HANDLE;
  VkDescriptorSet set = VK_NULL_HANDLE;
};

enum class DrawOrder { FrontToBack, BackToFront };

enum class Variant {
  EarlyZFrontToBack,
  EarlyZBackToFront,
  WriteDepthFrontToBack,
  WriteDepthBackToFront,
};

struct PipelineState {
  VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
  VkCompareOp depthCompare = VK_COMPARE_OP_NEVER;
  bool depthWrite = false;
  std::string fragShader;
  DrawOrder drawOrder = DrawOrder::FrontToBack;
};

VkRenderPass createRenderPass(const Device& dev, bool withDepth = false);
void destroyRenderPass(const Device& dev, VkRenderPass renderPass);

Descriptors createDescriptors(const Device& dev, const Buffer& uniform);
Descriptors createDescriptors(const Device& dev, const Buffer& uniform,
                              const Buffer& storage);
void destroyDescriptors(const Device& dev, Descriptors& descs);

VkPipelineLayout createPipelineLayout(const Device& dev,
                                      const Descriptors& descs,
                                      std::uint32_t pushSize = 0);
VkShaderModule createShaderModule(const Device& dev, const std::string& name);

VkPipeline createPipeline(const Device& dev, VkRenderPass renderPass,
                          VkPipelineLayout layout, VkShaderModule vertShader,
                          VkShaderModule fragShader, const Target& target,
                          const PipelineState& state = PipelineState{});

#endif
