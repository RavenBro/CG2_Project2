#version 330 core

layout (location = 0) out vec2 vz;
uniform vec4 material;
in vec3 view_pos;
void main()
{
    float z = sqrt(length(view_pos)/100);
    vz = vec2(z, material.w);
}