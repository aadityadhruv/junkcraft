#version 410 core
out vec4 frag_colour;
uniform vec3 face_colors[6];
void main() {
  frag_colour = vec4( face_colors[gl_PrimitiveID/2], 1.0 );
};
