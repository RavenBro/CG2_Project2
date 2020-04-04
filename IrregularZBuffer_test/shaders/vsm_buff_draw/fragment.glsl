#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D vsm_tex;

void main() 
{   
    vec4 c = texture(vsm_tex, UV);
    color = vec4(abs(c.r), abs(c.g), abs(c.b), 1.0);
}