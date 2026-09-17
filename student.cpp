#include "utils/Student.h"
#include <glm/gtc/matrix_transform.hpp>

#include <cstring>

const std::vector<Vertex> TRIANGLE = {
    {{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
};

VkVertexInputBindingDescription Vertex::bindingDescription() {
  VkVertexInputBindingDescription desc{};
  desc.binding = 0;
  desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  desc.stride = 20;
  return desc;
}

std::vector<VkVertexInputAttributeDescription> Vertex::attributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attrs(2);

    attrs[0].binding  = 0;
    attrs[0].location = 0;
    attrs[0].format   = VK_FORMAT_R32G32_SFLOAT;
    attrs[0].offset   = 0;

    attrs[1].binding  = 0;
    attrs[1].location = 1;
    attrs[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attrs[1].offset   = 8;

    return attrs;
}

struct Params {
    glm::mat4     mvp;
    std::uint32_t costLoops;
    std::uint32_t stripeWidth;
    std::uint32_t _pad[2];
};

static_assert(sizeof(Params) == 80,
              "Params must match the std140 table in the handout");

std::vector<std::uint8_t> uniformBlock(std::uint32_t costLoops,
                                       std::uint32_t stripeWidth) {
    Params params{};
    params.mvp         = glm::mat4(1.0f);
    params.costLoops   = costLoops;
    params.stripeWidth = stripeWidth;

    std::vector<std::uint8_t> bytes(sizeof(Params));
    std::memcpy(bytes.data(), &params, sizeof(Params));
    return bytes;
}

PipelineState pipelineState(Variant v) {
  PipelineState s{};

  // TASK 3a: TRIANGLE is wound counter-clockwise in framebuffer coordinates
  s.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

  // TASK 3b
  s.cullMode = VK_CULL_MODE_BACK_BIT;

  // TASK 3c: reversed-Z -- the pass clears depth to 0.0, so the larger depth wins
  s.depthCompare = VK_COMPARE_OP_GREATER;

  // TASK 3d
  s.depthWrite = true;

  const bool earlyZ = (v == Variant::EarlyZFrontToBack) ||
                      (v == Variant::EarlyZBackToFront);
  const bool frontToBack = (v == Variant::EarlyZFrontToBack) ||
                           (v == Variant::WriteDepthFrontToBack);

  // TASK 3e: A allows the depth test to run before the shader, B writes gl_FragDepth
  s.fragShader = earlyZ ? "earlyz_a.frag" : "earlyz_b.frag";

  // TASK 3f
  s.drawOrder = frontToBack ? DrawOrder::FrontToBack : DrawOrder::BackToFront;

  return s;
}

const std::vector<Vertex> QUAD = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
    {{ 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
};

const std::vector<uint16_t> QUAD_INDICES = {
    0, 1, 2,
    0, 2, 3,
};

std::vector<glm::mat4> instanceBuffer() {
    std::vector<glm::mat4> transforms;
    transforms.reserve(INSTANCES);

    const int cols = 100, rows = 50;
    const float s = 0.01f;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float x = (c + 0.5f) / cols * 2.0f - 1.0f;
            float y = (r + 0.5f) / rows * 2.0f - 1.0f;

            glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f))
                        * glm::scale(glm::mat4(1.0f), glm::vec3(s, s, 1.0f));
            transforms.push_back(m);
        }
    }
    return transforms;
}

void recordDraw(VkCommandBuffer cmd, uint32_t indexCount) {
    vkCmdDrawIndexed(cmd, indexCount, INSTANCES, 0, 0, 0);
}
