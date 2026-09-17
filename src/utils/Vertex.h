#ifndef __VERTEX_H_INCLUDED__
#define __VERTEX_H_INCLUDED__

#include <vulkan/vulkan.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription bindingDescription();
  static std::vector<VkVertexInputAttributeDescription> attributeDescriptions();
};

static_assert(sizeof(Vertex) == 20, "Vertex must be 20 bytes");
static_assert(offsetof(Vertex, pos) == 0, "pos must start at byte 0");
static_assert(offsetof(Vertex, color) == 8, "color must start at byte 8");

extern const std::vector<Vertex> TRIANGLE;

#endif
