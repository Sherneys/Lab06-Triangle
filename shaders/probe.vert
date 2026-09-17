#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform Params {
  mat4 mvp;
  uint costLoops;
  uint stripeWidth;
} u;

layout(set = 0, binding = 1) readonly buffer Instances {
  mat4 transforms[];
} instances;

layout(push_constant) uniform Scene {
  float scale;
  uint  fetches;
  uint  useInstances;
} scene;

void main() {
  mat4 model = scene.useInstances != 0u ? instances.transforms[gl_InstanceIndex] : mat4(1.0);
  gl_Position = u.mvp * model * vec4(inPosition * scene.scale, 0.0, 1.0);
  fragColor   = inColor;
}
