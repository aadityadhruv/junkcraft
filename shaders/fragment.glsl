#version 410 core
out vec4 frag_colour;
uniform vec3 face_colors[6];
uniform vec3 light_color;
in vec3 normal;
in vec3 frag_pos;
in vec2 text_coord;
uniform sampler2D block_texture;
void main() {
    vec3 ambient_color = vec3(0.1);
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(vec3(1.0f, 2.0f, 1.0f));
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    vec4 final_texture = texture(block_texture, text_coord);
    vec4 lighting = vec4(ambient_color + diffuse, 1.0f); 
    frag_colour =  lighting * final_texture;
};
