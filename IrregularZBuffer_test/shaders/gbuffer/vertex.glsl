#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal_model;
layout (location = 2) in vec2 uv;
const int NUM_CASCADES = 3;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 light_NDC = mat4(1.f);
uniform mat4 near_cascade_light;
uniform mat4 middle_cascade_light;
uniform mat4 far_cascade_light;

out vec4 LightSpacePos[NUM_CASCADES];

out vec2 uv_ndc;
out vec3 vert_cam;
out vec3 normal;
out vec4 coords_for_shades;
out vec4 world_pos;
void main()
{
    vec4 pos =model*vec4(vertex, 1.f);
    LightSpacePos[0] = near_cascade_light * pos;
    LightSpacePos[1] = middle_cascade_light * pos;
    LightSpacePos[2] = far_cascade_light * pos;
    
    world_pos = pos;
    coords_for_shades = pos;
    vec4 v = view * coords_for_shades;
    coords_for_shades = light_NDC * coords_for_shades;
    gl_Position = projection * v;
    vert_cam = vec3(v);
    uv_ndc = uv;
    //normal = mat3(transpose(inverse(view*model))) * normal_model;
    normal = mat3(transpose(inverse(view))) * mat3(transpose(inverse(model))) * normal_model;
}