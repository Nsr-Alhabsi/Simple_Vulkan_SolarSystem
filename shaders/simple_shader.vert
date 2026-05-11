#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color; 
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragColor; 
layout(location = 1) out vec2 fragUV;

layout(push_constant) uniform Push {
  mat4 transform;

  layout(offset = 64) vec3 color1;
  layout(offset = 80) vec3 color2;
  layout(offset = 96) vec2 gradDir;
  layout(offset = 104) int useGradient;
} push;

void main() {
  gl_Position = push.transform * vec4(position, 0.0, 1.0);
  fragColor = color;
  fragUV = uv;
}