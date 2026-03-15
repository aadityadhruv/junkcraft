#version 410 core
out vec4 frag_colour;
in vec2 text_coord;
uniform float opacity;
uniform sampler2D obj_texture;
void main() {
    vec4 final_texture = texture(obj_texture, text_coord);
    if (final_texture.a < 0.1)  {
        discard;
    }
    frag_colour = vec4(final_texture.xyz, opacity);
}
