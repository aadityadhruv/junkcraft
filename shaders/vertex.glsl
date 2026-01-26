#version 410 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 col;
flat out vec3 color;
uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
void main() {
  gl_Position = perspective * model * vec4( pos, 1.0 );
  color = col;
};
