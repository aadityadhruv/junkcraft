#version 410 core
out vec4 frag_colour;
uniform vec3 face_colors[6];
uniform vec3 light_color;
in vec3 normal;
in vec3 frag_pos;
void main() {
    vec3 ambient_color = vec3(0.1);
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(vec3(1.0f, 1.0f, 1.0f));
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    frag_colour = vec4((ambient_color + diffuse) * face_colors[gl_PrimitiveID/2], 1.0);
};
