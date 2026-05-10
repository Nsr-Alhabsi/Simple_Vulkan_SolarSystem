#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
  mat2 transform;
  vec2 offset;
  vec3 color1;
  vec3 color2;
  vec2 gradDir;
  bool useGradient;
} push;

void main() {
  if (push.useGradient) {
    float t = dot(fragUV, normalize(push.gradDir));
    t = clamp(t + 0.5, 0.0, 1.0);

    outColor = vec4(mix(push.color1, push.color2, t), 1.0);
  } else {
    outColor = vec4(push.color, 1.0);
  }
}