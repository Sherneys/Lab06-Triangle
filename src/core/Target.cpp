#include "core/Target.h"

#include <cstring>

#include "core/Buffer.h"

static void createImage(const Device& dev, std::uint32_t width,
                        std::uint32_t height, VkFormat format,
                        VkImageUsageFlags usage, VkImageAspectFlags aspect,
                        VkImage& image, VkDeviceMemory& memory,
                        VkImageView& view) {
  VkImageCreateInfo imageCI{};
  imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCI.imageType = VK_IMAGE_TYPE_2D;
  imageCI.format = format;
  imageCI.extent = {width, height, 1};
  imageCI.mipLevels = 1;
  imageCI.arrayLayers = 1;
  imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCI.usage = usage;
  imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  check(vkCreateImage(dev.device, &imageCI, nullptr, &image), "vkCreateImage");

  VkMemoryRequirements memReq;
  vkGetImageMemoryRequirements(dev.device, image, &memReq);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReq.size;
  allocInfo.memoryTypeIndex = findMemoryType(
      dev, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  check(vkAllocateMemory(dev.device, &allocInfo, nullptr, &memory),
        "vkAllocateMemory");
  check(vkBindImageMemory(dev.device, image, memory, 0), "vkBindImageMemory");

  VkImageViewCreateInfo viewCI{};
  viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCI.image = image;
  viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCI.format = format;
  viewCI.subresourceRange = {aspect, 0, 1, 0, 1};
  check(vkCreateImageView(dev.device, &viewCI, nullptr, &view),
        "vkCreateImageView");
}

Target createTarget(const Device& dev, VkRenderPass renderPass,
                    std::uint32_t width, std::uint32_t height, bool withDepth) {
  Target target{};
  target.width = width;
  target.height = height;

  createImage(
      dev, width, height, TARGET_FORMAT,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
      VK_IMAGE_ASPECT_COLOR_BIT, target.handle, target.memory, target.view);

  std::vector<VkImageView> attachments = {target.view};
  if (withDepth) {
    createImage(dev, width, height, DEPTH_FORMAT,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT, target.depth, target.depthMemory,
                target.depthView);
    attachments.push_back(target.depthView);
  }

  VkFramebufferCreateInfo fbCI{};
  fbCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fbCI.renderPass = renderPass;
  fbCI.attachmentCount = static_cast<std::uint32_t>(attachments.size());
  fbCI.pAttachments = attachments.data();
  fbCI.width = width;
  fbCI.height = height;
  fbCI.layers = 1;
  check(vkCreateFramebuffer(dev.device, &fbCI, nullptr, &target.framebuffer),
        "vkCreateFramebuffer");
  return target;
}

void destroyTarget(const Device& dev, Target& target) {
  vkDestroyFramebuffer(dev.device, target.framebuffer, nullptr);
  if (target.depth != VK_NULL_HANDLE) {
    vkDestroyImageView(dev.device, target.depthView, nullptr);
    vkDestroyImage(dev.device, target.depth, nullptr);
    vkFreeMemory(dev.device, target.depthMemory, nullptr);
  }
  vkDestroyImageView(dev.device, target.view, nullptr);
  vkDestroyImage(dev.device, target.handle, nullptr);
  vkFreeMemory(dev.device, target.memory, nullptr);
  target = Target{};
}

std::vector<std::uint8_t> readTarget(const Device& dev, const Target& target) {
  VkDeviceSize bytes =
      static_cast<VkDeviceSize>(target.width) * target.height * 4;
  Buffer staging = createBuffer(dev, bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  check(vkBeginCommandBuffer(dev.cmd, &beginInfo), "vkBeginCommandBuffer");

  VkBufferImageCopy region{};
  region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  region.imageExtent = {target.width, target.height, 1};
  vkCmdCopyImageToBuffer(dev.cmd, target.handle,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging.handle,
                         1, &region);
  check(vkEndCommandBuffer(dev.cmd), "vkEndCommandBuffer");

  VkSubmitInfo submit{};
  submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &dev.cmd;
  check(vkQueueSubmit(dev.gfxQueue, 1, &submit, VK_NULL_HANDLE),
        "vkQueueSubmit");
  check(vkQueueWaitIdle(dev.gfxQueue), "vkQueueWaitIdle");

  std::vector<std::uint8_t> rgba(static_cast<std::size_t>(bytes));
  std::memcpy(rgba.data(), staging.mapped, rgba.size());
  destroyBuffer(dev, staging);
  return rgba;
}
