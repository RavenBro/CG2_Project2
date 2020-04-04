#version 330 core

layout (location = 0) out vec3 vz;
uniform vec4 material;
void main()
{
    float z = gl_FragCoord.z;
    vz = vec3(z, z*z, material.w);
}