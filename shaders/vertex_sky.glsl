#version 410 core

layout(location=0) in vec3 pos;
layout(location=1) in vec2 i_text_coord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
uniform vec2 texture_scale;
out vec3 frag_pos;
out vec2 text_coord;
void main() {
  gl_Position = perspective*view*model*vec4(pos, 1.0);
  text_coord = texture_scale * i_text_coord;
  frag_pos = vec3(model*vec4(pos, 1.0));

}
