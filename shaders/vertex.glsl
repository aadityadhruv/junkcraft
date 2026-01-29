#version 410 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 v_normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
out vec3 normal;
out vec3 frag_pos;
void main() {
  gl_Position = perspective*view*model*vec4(pos, 1.0);
  normal = v_normal;
  frag_pos = vec3(model*vec4(pos, 1.0));

};
