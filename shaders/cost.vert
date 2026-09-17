#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

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
  vec2 cell = vec2(float(gl_InstanceIndex % scene.columns),
                   float(gl_InstanceIndex / scene.columns));
  vec2 step = vec2(2.0) / float(scene.columns);
  vec2 pos  = inPosition * scene.scale + cell * step - vec2(1.0);

  float x = inColor.r;
  if (scene.vertexLoop != 0u) {
    for (uint i = 0u; i < u.costLoops; i++) { x = x * 1.000001 + 1.0; }
  }

  gl_Position = u.mvp * vec4(pos, 0.0, 1.0);
  fragColor   = inColor * clamp(x * 1e-9, 0.0, 1.0) + inColor;
}
