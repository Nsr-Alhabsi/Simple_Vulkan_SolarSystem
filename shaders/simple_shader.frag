#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat4 transform;

    layout(offset = 64) vec3 color1;
    layout(offset = 80) vec3 color2;
    layout(offset = 96) vec2 gradDir;
    layout(offset = 104) int useGradient;
} push;

void main() {
  float distance = sqrt(dot(fragUV - 0.5, fragUV - 0.5));
  
  if (distance > 0.5) {
    discard;
  }

  if (push.useGradient == 1) {
    float t = clamp(dot(fragUV - 0.5, normalize(push.gradDir)) + 0.5, 0.0, 1.0);
    vec3 finalColor = mix(push.color1, push.color2, t);
    outColor = vec4(finalColor, 1.0);
  } else {
    outColor = vec4(push.color1, 1.0);
  }
}