#version 330 core

#define MAX_LIGHT_COUNT 16
#define MAX_SM_LIGHT_COUNT 4
#define MAX_VSM_LIGHT_COUNT 2
#define MAX_CM_LIGHT_COUNT 2

in vec2 uv;

uniform mat4 InverseCamM;
uniform sampler2D normalTex;
uniform sampler2D vertexTex;

uniform samplerCube specMap;
uniform sampler2D brdfLuT;

void main()
{   
    vec3 N = vec3(texture(normalTex, uv));
    N = normalize(N);
    vec3 pos = vec3(texture(vertexTex, uv));
    vec3 worldPos = (InverseCamM * vec4(pos, 1)).xyz;
    vec3 V = normalize(cameraPos - worldPos);

    vec4 material = texture(materialTex, uv);
    float roughness = material.r;
    vec3 F0 = calcF0(albedo, metallic);

    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = (1.0 - kS) * (1 - metallic);
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
    vec3 R = reflect(-V, N);
    vec3 spec = textureLod(specMap, R, roughness * (SPEC_MIP_COUNT - 1)).rgb;
    vec2 brdf = texture(brdfLuT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    spec = spec * (kS * brdf.x + brdf.y);
}