#version 330 core

in vec2 uv_ndc;
in vec3 vert_cam;
in vec3 normal;
in vec3 vertWord;
in vec3 normalW;

uniform bool use_diffuse_tex = true;
uniform sampler2D diffuse_tex;


uniform bool use_normal_tex = false;
uniform sampler2D normal_tex;
uniform bool usePBRTex = false;
uniform sampler2D pbrTex;

uniform float metallic = 0.f;
uniform float roughness = 0.f;
uniform float reflection = 0.f;
uniform vec4 color = vec4(1);

layout (location = 0) out vec4 diffuse_color;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec3 camera_v;
layout (location = 3) out vec4 material;
layout (location = 4) out vec4 outWorldVert;
layout (location = 5) out vec3 worldNormal;

void main()
{
    worldNormal = normalize(normalW);
    outWorldVert = vec4(vertWord, 1.f);
    diffuse_color = (use_diffuse_tex)? vec4(pow(texture(diffuse_tex, uv_ndc).rgb, vec3(2.2)), 1.0) : color;

    camera_v = vert_cam;
    out_normal = normalize(normal); 
    if(usePBRTex){
        vec3 m = texture(pbrTex, uv_ndc).rgb;
        material = vec4(m.y, m.z, m.y, 1.f);
    } else {
        material = vec4(roughness, metallic, reflection, 1.f);
    }
}