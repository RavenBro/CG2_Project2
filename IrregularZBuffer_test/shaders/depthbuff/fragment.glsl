#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D depthTex;

void main()
{
    float z = texture(depthTex, UV).r;
    color = vec4(vec3(z), 1.0);
}