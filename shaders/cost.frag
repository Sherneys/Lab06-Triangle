#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform Params {
  mat4 mvp;
  uint costLoops;
  uint stripeWidth;
} u;

layout(push_constant) uniform Scene {
  float scale;
  uint  columns;
  uint  vertexLoop;
  uint  fragmentLoop;
} scene;

void main() {
  float x = fragColor.r;
  if (scene.fragmentLoop != 0u) {
    for (uint i = 0u; i < u.costLoops; i++) { x = x * 1.000001 + 1.0; }
  }
  outColor = vec4(fragColor * clamp(x * 1e-9, 0.0, 1.0) + fragColor, 1.0);
}
