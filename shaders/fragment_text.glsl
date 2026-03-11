#version 410 core
in vec2 tex_coords;
out vec4 color;
uniform sampler2D text;

void main()
{    
    if (texture(text, tex_coords).r < 0.1)  {
        discard;
    }
    color = vec4(1.0f);
}
