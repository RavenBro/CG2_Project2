#version 330 core

#define SAMPLES_COUNT 50
#define MAX_RSM_LIGHT_COUNT 1
uniform vec2 rsmSamples[SAMPLES_COUNT];

uniform sampler2D worldTex;
uniform sampler2D worldNormTex;

struct RSMLight {
    mat4 viewProj;
    sampler2D worldTex;
    sampler2D fluxTex;
    sampler2D normalTex;
};

uniform RSMLight lights[MAX_RSM_LIGHT_COUNT];

vec3 RSMLightIrr(vec3 wordVert, vec3 wordNorm, in mat4 lightMat, in sampler2D normT, in sampler2D fluxT, in sampler2D posT);

in vec2 uv;
out vec4 color;

void main() {
    vec3 pos = texture(worldTex, uv).xyz;
    vec3 norm = texture(worldNormTex, uv).xyz;
    color = vec4(0, 0, 0, 1);
    if(length(norm) == 0) {
        return; 
    }
    for(int i = 0; i < MAX_RSM_LIGHT_COUNT; i++){
        color += vec4(RSMLightIrr(pos, norm, lights[i].viewProj, 
            lights[i].normalTex, lights[i].fluxTex, lights[i].worldTex), 1.0);
    }
}

const float rsmR = 0.04;
const float rsmCoef = 10;

vec3 RSMLightIrr(vec3 wordVert, vec3 wordNorm, in mat4 lightMat, 
    in sampler2D normT, in sampler2D fluxT, in sampler2D posT)
{
    vec4 lightV = lightMat * vec4(wordVert, 1.0);
    vec3 ndc = lightV.xyz/lightV.w;
    ndc = ndc * 0.5 + 0.5;
    if(ndc.z <= 0 || ndc.z > 1.f || ndc.x < 0 || ndc.x > 1.f || ndc.y < 0 || ndc.y >= 1.f) return vec3(0);
    
    vec3 Lo = vec3(0);
    vec3 P = wordVert;
    vec3 N = wordNorm;

    for(int i = 0; i < SAMPLES_COUNT; i++){
        vec2 rnd = rsmSamples[i].xy;
        vec2 sample = ndc.xy + rsmR * rnd;

        vec3 pos = texture(posT, sample).xyz;
        vec3 norm = texture(normT, sample).xyz;
        vec3 flux = texture(fluxT, sample).xyz;

        vec3 dir = (P - pos);
        vec3 light = flux * max(0, dot(norm, P - pos)) * max(0, dot(N, pos - P)) / pow(length(P - pos), 4);
        light *= length(rnd) * length(rnd);
        Lo += light;
    }

    return Lo * rsmCoef / SAMPLES_COUNT;
}