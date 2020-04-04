#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal_model;
layout (location = 2) in vec2 uv;
//layout (location = 3) in vec2 n_uv;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
//uniform mat3 normal_mat = mat3(1);

out vec2 uv_ndc;
//out vec2 uv_normal;
out vec3 vert_cam;
out vec3 normal;
out vec3 vertWord;
out vec3 normalW;

void main()
{
    vertWord = (model * vec4(vertex, 1.f)).xyz;
    vec4 v = view * vec4(vertWord, 1.f);
    gl_Position = projection * v;
    vert_cam = vec3(v);
    uv_ndc = uv;

    normal = mat3(transpose(inverse(view*model))) * normal_model;
    normalW = mat3(transpose(inverse(model))) * normal_model;
}