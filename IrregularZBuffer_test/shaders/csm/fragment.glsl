#version 330 core

layout (location = 0) out vec2 vz1;
layout (location = 1) out vec2 vz2;
layout (location = 2) out vec2 vz3;
uniform int cascade=0;
void main()
{
    float z = gl_FragCoord.z;
    vec2 vz = vec2(z, z*z);
    vz=vec2(1.0,0.0);
    if (cascade==0) 
    {
        vz1=vz;
        return;
    }
    if (cascade==1) 
    {
        vz2=vz;
        return;
    }
    if (cascade==2) 
    {
        vz3=vz;
        return;
    }
}