#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
  mat2 transform;
  vec2 offset;
  vec3 color;
  bool useGradient;
} push;

void main() {
  if (push.useGradient) {
    outColor = vec4(fragColor, 1.0);
  } else {
    outColor = vec4(push.color, 1.0);
  }
}