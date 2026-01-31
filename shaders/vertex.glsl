#version 410 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 v_normal;
layout(location=2) in vec2 i_text_coord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
out vec3 normal;
out vec3 frag_pos;
out vec2 text_coord;
void main() {
  gl_Position = perspective*view*model*vec4(pos, 1.0);
  normal = v_normal;
  text_coord = i_text_coord;
  frag_pos = vec3(model*vec4(pos, 1.0));

};
