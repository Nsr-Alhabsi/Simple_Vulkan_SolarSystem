#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
  mat2 transform;
  vec2 offset;

  layout(offset = 32) vec3 color1;
  layout(offset = 48) vec3 color2;
  layout(offset = 64) vec2 gradDir;
  layout(offset = 72) bool useGradient;
} push;

void main() {
  if (push.useGradient) {
    float t = dot(fragUV - 0.5, normalize(push.gradDir));
    t = clamp(t + 0.5, 0.0, 1.0);
    
    outColor = vec4(mix(push.color1.rgb, push.color2.rgb, t), 1.0);
  } else {
    outColor = vec4(push.color1.rgb, 1.0);
  }
}