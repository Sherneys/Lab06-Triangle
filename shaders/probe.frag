#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform Params {
  mat4 mvp;
  uint costLoops;
  uint stripeWidth;
} u;

layout(set = 0, binding = 1) readonly buffer Bulk {
  vec4 data[];
} bulk;

layout(push_constant) uniform Scene {
  float scale;
  uint  fetches;
  uint  useInstances;
} scene;

void main() {
  float x = fragColor.r;
  for (uint i = 0u; i < u.costLoops; i++) { x = x * 1.000001 + 1.0; }

  vec4 fetched = vec4(0.0);
  for (uint i = 0u; i < scene.fetches; i++) {
    uint index = (uint(gl_FragCoord.x) * 7u + uint(gl_FragCoord.y) * 1031u + i * 4099u) % 20000u;
    fetched += bulk.data[index];
  }

  outColor = vec4(fragColor * clamp(x * 1e-9, 0.0, 1.0) + fragColor + fetched.rgb * 1e-9, 1.0);
}
