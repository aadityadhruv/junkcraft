#version 410 core
out vec4 frag_colour;
flat in vec3 color;
void main() {
  frag_colour = vec4( color, 1.0 );
};
