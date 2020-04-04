#version 330 core
in vec4 cl;
layout (location = 6) out vec4 color;
void main() 
{   
    //float t = 30*(0.2-cl.w);
    color = cl;
    //color = vec4(1,0,0,1);
}