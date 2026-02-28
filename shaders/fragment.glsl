#version 410 core
out vec4 frag_colour;
uniform vec3 light_color;
in vec3 normal;
in vec3 frag_pos;
in vec2 text_coord;
uniform sampler2D block_texture;
void main() {
    vec3 top_ambient = vec3(0.5);
    vec3 ambient_dir = vec3(0.0f, 1.0f, 0.0f);
    vec3 bottom_ambient = vec3(0.4);
    float value = dot(normal, ambient_dir) * 0.5 + 0.3;
    vec3 inter = mix(bottom_ambient,top_ambient,value);
    vec3 ambient_color = inter;
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(vec3(1.0f, 2.0f, 1.0f));
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    vec4 final_texture = texture(block_texture, text_coord);
    if (final_texture.a < 0.1)  {
        discard;
    }
    vec4 lighting = vec4(ambient_color + diffuse, 1.0f); 
    frag_colour =  lighting * final_texture;
}
