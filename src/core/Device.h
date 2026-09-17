#ifndef __DEVICE_H_INCLUDED__
#define __DEVICE_H_INCLUDED__

#include "utils/utils.h"

struct GLFWwindow;

struct Device {
  VkInstance       instance = VK_NULL_HANDLE;
  VkSurfaceKHR     surface  = VK_NULL_HANDLE;
  VkPhysicalDevice gpu      = VK_NULL_HANDLE;
  VkDevice         device   = VK_NULL_HANDLE;
  VkQueue          gfxQueue = VK_NULL_HANDLE;
  std::uint32_t    gfxFamily = 0;

  VkCommandPool   cmdPool = VK_NULL_HANDLE;
  VkCommandBuffer cmd     = VK_NULL_HANDLE;
  VkQueryPool     queries = VK_NULL_HANDLE;

  VkPhysicalDeviceProperties props{};
  std::uint32_t subgroupSize      = 0;
  bool          hasTimestamps     = false;
  bool          hasPipelineStats  = false;
};

Device createDevice(GLFWwindow* window);
void   destroyDevice(Device& dev);

std::uint32_t findMemoryType(const Device& dev, std::uint32_t typeFilter, VkMemoryPropertyFlags props);

#endif
