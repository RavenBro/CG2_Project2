#version 330 core

in vec2 UV;
in vec3 vNormal;
in vec4 vModel;
 

uniform sampler2D diffuse_tex;
uniform sampler2DShadow shadow_tex;
uniform sampler2D vsm_tex;

uniform bool use_diffuse_tex = false;
uniform bool use_shadow_tex = false;

uniform vec3 default_color = vec3(1.f);

layout (location = 0) out vec4 color;
layout (location = 1) out vec3 norm;

float Shade(vec4 modelVert)
{
    vec3 ndc = modelVert.xyz / modelVert.w;
    ndc = 0.5*ndc + 0.5;
    ndc.z -= 0.003f;
    if(ndc.z > 1.f) return 0.f;
    return texture(shadow_tex, ndc);
}

float ShadeVSM(vec4 modelVert)
{
    vec3 ndc = modelVert.xyz / modelVert.w;
    ndc = 0.5*ndc + 0.5;
    if(ndc.z > 1.f) return 0.f;

    vec2 vsm = texture(vsm_tex, ndc.xy).rg;
    float mu = vsm.x;
    float s2 = vsm.y - mu*mu;
    
    return 1.f - s2 / (s2 + (ndc.z - mu)*(ndc.z - mu));
}

void main()
{
    vec3 diffuse_color = use_diffuse_tex? texture(diffuse_tex, UV).rgb : default_color;
    float shade = use_shadow_tex? ShadeVSM(vModel) : 0.f;
    color = vec4((1.f - shade * 0.8f) * diffuse_color, 1.f);
    norm= vNormal;
}   