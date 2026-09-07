#version 410 core
out vec4 frag_colour;
uniform vec3 light_color;
in vec3 normal;
in vec3 frag_pos;
in vec2 text_coord;
uniform vec3 player_position;
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
    vec3 diffuse = diff * light_color;
    vec4 final_texture = texture(block_texture, text_coord);
    if (final_texture.a < 0.1)  {
        discard;
    }
    vec4 lighting = vec4(ambient_color + diffuse, 1.0f); 
    vec4 point_color = lighting * final_texture;

    float distance_to_vertex = length(player_position - frag_pos);
    float fog_scale = 0.01;
    float fog_intensity = pow(distance_to_vertex * fog_scale, 4);
    float fog_amount = clamp(fog_intensity, 0.0f, 1.0f);
    vec3  fog_color = vec3(0.5,0.6,0.7);
    vec3 fog_point_color = mix(point_color.xyz, fog_color, fog_amount);

    frag_colour = vec4(fog_point_color, point_color.a);
}
