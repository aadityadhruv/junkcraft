#version 410 core

layout(location=0) in vec2 pos;
layout(location=2) in vec2 i_text_coord;
uniform mat4 model;
uniform mat4 projection;
out vec2 text_coord;
void main() {
  gl_Position = projection*model*vec4(pos, 0.0, 1.0);
  text_coord = i_text_coord;
}
