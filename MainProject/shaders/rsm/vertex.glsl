#version 330 core

layout (location = 0) in vec3 vertex_M;
layout (location = 1) in vec3 normal_M;
layout (location = 2) in vec2 uv_M;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertexWorld;
out vec3 normalWorld;
out vec2 uv;

void main()
{
    uv = uv_M;
    mat3 Nw = mat3(transpose(inverse(model)));

    vertexWorld = vec3(model * vec4(vertex_M, 1.0));
    normalWorld = Nw * normal_M;
    gl_Position = projection * view * vec4(vertexWorld, 1.0);
}