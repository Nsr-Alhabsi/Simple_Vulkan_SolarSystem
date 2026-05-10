#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color; 
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragColor; 
layout(location = 1) out vec2 fragUV;

layout(push_constant) uniform Push {
  mat2 transform;
  vec2 offset;

  layout(offset = 16) vec3 color1;
  layout(offset = 32) vec3 color2;
  layout(offset = 48) vec2 gradDir;
  layout(offset = 56) bool useGradient;
} push;

void main() {
  gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
  fragColor = color;
  fragUV = uv;
}