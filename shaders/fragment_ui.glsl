#version 410 core
out vec4 frag_colour;
uniform vec4 color;
uniform sampler2D block_texture;
uniform int enable_texture;
in vec2 text_coord;
void main() {
    if (enable_texture == 1) {
        vec4 final_texture = texture(block_texture, text_coord);
        if (final_texture.a < 0.1)  {
            discard;
        }
        frag_colour =  vec4(color) * final_texture;
    } else {
        frag_colour =  color;
    }
}
