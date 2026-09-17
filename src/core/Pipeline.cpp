#include "core/Pipeline.h"

#include "utils/Vertex.h"

VkRenderPass createRenderPass(const Device& dev, bool withDepth) {
  VkAttachmentDescription color{};
  color.format = TARGET_FORMAT;
  color.samples = VK_SAMPLE_COUNT_1_BIT;
  color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

  VkAttachmentReference colorRef{};
  colorRef.attachment = 0;
  colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depth{};
  depth.format = DEPTH_FORMAT;
  depth.samples = VK_SAMPLE_COUNT_1_BIT;
  depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthRef{};
  depthRef.attachment = 1;
  depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription attachments[2] = {color, depth};

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorRef;
  subpass.pDepthStencilAttachment = withDepth ? &depthRef : nullptr;

  VkRenderPassCreateInfo renderPassCI{};
  renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCI.attachmentCount = withDepth ? 2u : 1u;
  renderPassCI.pAttachments = attachments;
  renderPassCI.subpassCount = 1;
  renderPassCI.pSubpasses = &subpass;

  VkRenderPass renderPass = VK_NULL_HANDLE;
  check(vkCreateRenderPass(dev.device, &renderPassCI, nullptr, &renderPass),
        "vkCreateRenderPass");
  return renderPass;
}

void destroyRenderPass(const Device& dev, VkRenderPass renderPass) {
  vkDestroyRenderPass(dev.device, renderPass, nullptr);
}

Descriptors createDescriptors(const Device& dev, const Buffer& uniform) {
  Descriptors descs{};

  VkDescriptorSetLayoutBinding binding{};
  binding.binding = 0;
  binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  binding.descriptorCount = 1;
  binding.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo layoutCI{};
  layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutCI.bindingCount = 1;
  layoutCI.pBindings = &binding;
  check(vkCreateDescriptorSetLayout(dev.device, &layoutCI, nullptr,
                                    &descs.layout),
        "vkCreateDescriptorSetLayout");

  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolCI{};
  poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCI.maxSets = 1;
  poolCI.poolSizeCount = 1;
  poolCI.pPoolSizes = &poolSize;
  check(vkCreateDescriptorPool(dev.device, &poolCI, nullptr, &descs.pool),
        "vkCreateDescriptorPool");

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descs.pool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &descs.layout;
  check(vkAllocateDescriptorSets(dev.device, &allocInfo, &descs.set),
        "vkAllocateDescriptorSets");

  VkDescriptorBufferInfo bufInfo{};
  bufInfo.buffer = uniform.handle;
  bufInfo.offset = 0;
  bufInfo.range = uniform.size;

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = descs.set;
  write.dstBinding = 0;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write.pBufferInfo = &bufInfo;
  vkUpdateDescriptorSets(dev.device, 1, &write, 0, nullptr);
  return descs;
}

Descriptors createDescriptors(const Device& dev, const Buffer& uniform,
                              const Buffer& storage) {
  Descriptors descs{};

  VkDescriptorSetLayoutBinding bindings[2]{};
  bindings[0].binding = 0;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[1].binding = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  bindings[1].descriptorCount = 1;
  bindings[1].stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo layoutCI{};
  layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutCI.bindingCount = 2;
  layoutCI.pBindings = bindings;
  check(vkCreateDescriptorSetLayout(dev.device, &layoutCI, nullptr,
                                    &descs.layout),
        "vkCreateDescriptorSetLayout");

  VkDescriptorPoolSize poolSizes[2]{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = 1;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[1].descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolCI{};
  poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCI.maxSets = 1;
  poolCI.poolSizeCount = 2;
  poolCI.pPoolSizes = poolSizes;
  check(vkCreateDescriptorPool(dev.device, &poolCI, nullptr, &descs.pool),
        "vkCreateDescriptorPool");

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descs.pool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &descs.layout;
  check(vkAllocateDescriptorSets(dev.device, &allocInfo, &descs.set),
        "vkAllocateDescriptorSets");

  VkDescriptorBufferInfo bufInfos[2]{};
  bufInfos[0].buffer = uniform.handle;
  bufInfos[0].range = uniform.size;
  bufInfos[1].buffer = storage.handle;
  bufInfos[1].range = storage.size;

  VkWriteDescriptorSet writes[2]{};
  writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[0].dstSet = descs.set;
  writes[0].dstBinding = 0;
  writes[0].descriptorCount = 1;
  writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writes[0].pBufferInfo = &bufInfos[0];
  writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[1].dstSet = descs.set;
  writes[1].dstBinding = 1;
  writes[1].descriptorCount = 1;
  writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  writes[1].pBufferInfo = &bufInfos[1];
  vkUpdateDescriptorSets(dev.device, 2, writes, 0, nullptr);
  return descs;
}

void destroyDescriptors(const Device& dev, Descriptors& descs) {
  vkDestroyDescriptorPool(dev.device, descs.pool, nullptr);
  vkDestroyDescriptorSetLayout(dev.device, descs.layout, nullptr);
  descs = Descriptors{};
}

VkPipelineLayout createPipelineLayout(const Device& dev,
                                      const Descriptors& descs,
                                      std::uint32_t pushSize) {
  VkPushConstantRange push{};
  push.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push.size = pushSize;

  VkPipelineLayoutCreateInfo layoutCI{};
  layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutCI.setLayoutCount = 1;
  layoutCI.pSetLayouts = &descs.layout;
  layoutCI.pushConstantRangeCount = pushSize > 0 ? 1u : 0u;
  layoutCI.pPushConstantRanges = &push;

  VkPipelineLayout layout = VK_NULL_HANDLE;
  check(vkCreatePipelineLayout(dev.device, &layoutCI, nullptr, &layout),
        "vkCreatePipelineLayout");
  return layout;
}

VkShaderModule createShaderModule(const Device& dev, const std::string& name) {
  std::vector<char> code =
      readFile(std::string(SHADER_DIR) + "/" + name + ".spv");

  VkShaderModuleCreateInfo moduleCI{};
  moduleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  moduleCI.codeSize = code.size();
  moduleCI.pCode = reinterpret_cast<const std::uint32_t*>(code.data());

  VkShaderModule shader = VK_NULL_HANDLE;
  check(vkCreateShaderModule(dev.device, &moduleCI, nullptr, &shader),
        "vkCreateShaderModule");
  return shader;
}

VkPipeline createPipeline(const Device& dev, VkRenderPass renderPass,
                          VkPipelineLayout layout, VkShaderModule vertShader,
                          VkShaderModule fragShader, const Target& target,
                          const PipelineState& state) {
  VkVertexInputBindingDescription binding = Vertex::bindingDescription();
  std::vector<VkVertexInputAttributeDescription> attrs =
      Vertex::attributeDescriptions();

  VkPipelineShaderStageCreateInfo stages[2]{};
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vertShader;
  stages[0].pName = "main";
  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = fragShader;
  stages[1].pName = "main";

  VkPipelineVertexInputStateCreateInfo vertexInput{};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInput.vertexBindingDescriptionCount = 1;
  vertexInput.pVertexBindingDescriptions = &binding;
  vertexInput.vertexAttributeDescriptionCount =
      static_cast<std::uint32_t>(attrs.size());
  vertexInput.pVertexAttributeDescriptions = attrs.data();

  VkPipelineInputAssemblyStateCreateInfo assembly{};
  assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkViewport viewport{};
  viewport.width = static_cast<float>(target.width);
  viewport.height = static_cast<float>(target.height);
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.extent = {target.width, target.height};

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo raster{};
  raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster.polygonMode = VK_POLYGON_MODE_FILL;
  raster.cullMode = state.cullMode;
  raster.frontFace = state.frontFace;
  raster.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisample{};
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState blendAttachment{};
  blendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo blend{};
  blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend.attachmentCount = 1;
  blend.pAttachments = &blendAttachment;

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = state.depthCompare != VK_COMPARE_OP_NEVER;
  depthStencil.depthWriteEnable = state.depthWrite;
  depthStencil.depthCompareOp = state.depthCompare;
  depthStencil.maxDepthBounds = 1.0f;

  VkGraphicsPipelineCreateInfo pipelineCI{};
  pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCI.stageCount = 2;
  pipelineCI.pStages = stages;
  pipelineCI.pVertexInputState = &vertexInput;
  pipelineCI.pInputAssemblyState = &assembly;
  pipelineCI.pViewportState = &viewportState;
  pipelineCI.pRasterizationState = &raster;
  pipelineCI.pMultisampleState = &multisample;
  pipelineCI.pDepthStencilState = &depthStencil;
  pipelineCI.pColorBlendState = &blend;
  pipelineCI.layout = layout;
  pipelineCI.renderPass = renderPass;

  VkPipeline pipeline = VK_NULL_HANDLE;
  check(vkCreateGraphicsPipelines(dev.device, VK_NULL_HANDLE, 1, &pipelineCI,
                                  nullptr, &pipeline),
        "vkCreateGraphicsPipelines");
  return pipeline;
}
