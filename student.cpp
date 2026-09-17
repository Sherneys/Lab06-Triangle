#include "utils/Student.h"

#include <cstring>

const std::vector<Vertex> TRIANGLE = {
    // TODO(TASK 1a): three vertices. Check values are in Part I of the handout.
};

VkVertexInputBindingDescription Vertex::bindingDescription() {
  VkVertexInputBindingDescription desc{};
  desc.binding = 0;
  desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  desc.stride =
      0;  // TODO(TASK 1b): how many bytes from one vertex to the next?
  return desc;
}

std::vector<VkVertexInputAttributeDescription> Vertex::attributeDescriptions() {
  // TODO(TASK 1c): two attributes. Each needs binding, location, format and
  // offset.
  return {};
}

struct Params {
  std::uint32_t placeholder[20];  // TODO(TASK 2a): replace with three std140
                                  // members, padded
};

static_assert(sizeof(Params) == 80,
              "Params must match the std140 table in the handout");

std::vector<std::uint8_t> uniformBlock(std::uint32_t costLoops,
                                       std::uint32_t stripeWidth) {
  Params params{};
  // TODO(TASK 2b): an identity matrix, and the two knobs passed through.

  std::vector<std::uint8_t> bytes(sizeof(Params));
  std::memcpy(bytes.data(), &params, sizeof(Params));
  return bytes;
}

PipelineState pipelineState(Variant v) {
  PipelineState s{};
  // TODO(TASK 3a): match the winding you chose in TASK 1a
  // TODO(TASK 3b): cull back faces
  // TODO(TASK 3c): depth compare, using Lab 05's reversed-Z convention
  // TODO(TASK 3d): should this pipeline write depth?
  // TODO(TASK 3e): "earlyz_a.frag" or "earlyz_b.frag", chosen from v
  // TODO(TASK 3f): DrawOrder::FrontToBack or ::BackToFront, chosen from v
  return s;
}

const std::vector<Vertex> QUAD = {
    // TODO(TASK 4a): four corners. Check values are in Part V of the handout.
};

const std::vector<std::uint16_t> QUAD_INDICES = {
    // TODO(TASK 4b): six indices, two triangles, both wound like TASK 1a.
};

std::vector<glm::mat4> instanceBuffer() {
  // TODO(TASK 5a): INSTANCES transforms, glm::translate and glm::scale.
  return {};
}

void recordDraw(VkCommandBuffer cmd, std::uint32_t indexCount) {
  // TODO(TASK 5b): one vkCmdDrawIndexed, with an instance count.
}
