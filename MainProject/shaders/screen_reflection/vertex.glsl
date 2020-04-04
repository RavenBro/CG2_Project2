#version 330 core
layout (location = 0) in vec3 in_vert;
layout (location = 2) in vec2 in_uv;

out vec2 uv;

void main()
{
    gl_Position = vec4(in_vert, 1.f);
    uv = in_uv;
}