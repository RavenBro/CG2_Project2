#version 330 core

layout (location = 0) out vec2 vz;

void main()
{
    float z = gl_FragCoord.z;
    vz = vec2(z, z*z);
}