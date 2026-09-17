#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform Params {
  mat4 mvp;
  uint costLoops;
  uint stripeWidth;
} u;

layout(push_constant) uniform Layer {
  float depth;
  float scale;
} layer;

void main() {
  gl_Position = u.mvp * vec4(inPosition * layer.scale, layer.depth, 1.0);
  fragColor   = inColor;
}
