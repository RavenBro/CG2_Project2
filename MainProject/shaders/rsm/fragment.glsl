#version 330 core

in vec3 vertexWorld;
in vec2 uv;
in vec3 normalWorld;

uniform sampler2D albedoTex;
uniform bool useTex = true;
uniform vec3 modelColor = vec3(0.8, 0.8, 0.8);
uniform vec3 lightColor = vec3(1.f, 1.f, 1.f);
uniform vec3 lightPos;

layout (location = 0) out vec3 flux;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec3 pos;

void main()
{
    normal = normalize(normalWorld);
    pos = vertexWorld;
    
    vec3 dir = normalize(lightPos - pos);
    vec3 albedo = (useTex)? vec3(texture(albedoTex, uv)) : modelColor;
    flux = albedo * lightColor * max(0, dot(normal, dir));
}

