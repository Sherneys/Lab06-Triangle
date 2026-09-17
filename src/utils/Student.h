#ifndef __STUDENT_H_INCLUDED__
#define __STUDENT_H_INCLUDED__

#include "core/Pipeline.h"
#include "utils/Vertex.h"

#include <glm/glm.hpp>

inline constexpr std::uint32_t INSTANCES = 5000;

std::vector<std::uint8_t> uniformBlock(std::uint32_t costLoops, std::uint32_t stripeWidth);

PipelineState pipelineState(Variant v);

extern const std::vector<Vertex> QUAD;
extern const std::vector<std::uint16_t> QUAD_INDICES;

std::vector<glm::mat4> instanceBuffer();
void                   recordDraw(VkCommandBuffer cmd, std::uint32_t indexCount);

#endif
