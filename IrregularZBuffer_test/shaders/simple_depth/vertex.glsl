#version 330 core

layout (location = 0) in vec3 vertex;

uniform mat4 proj;
uniform mat4 MV;
out vec3 view_pos;
void main()
{
    vec4 tmp = MV*vec4(vertex, 1.0);
    view_pos = tmp.xyz;
    gl_Position = proj * tmp;
}