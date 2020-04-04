#version 330 core


uniform sampler2D frameTex;
uniform sampler2D reflectTex; 
uniform sampler2D reflectMask;
uniform sampler2D materialTex;
uniform sampler2D bloomTex;
uniform sampler2D indirectLight; 

uniform sampler2D IBLReflectTex;
uniform sampler2D FTex;
uniform sampler2D partTex;
uniform bool useReflections = false;
uniform bool useBloom = false;
uniform bool useIndirectLight = false;
in vec2 uv;

out vec4 color;

const float MAX_SSR_LOD = 4.f;
const float MAX_SSR_DIST = 4.f;
vec3 getSSR(in vec2 uv, float roughness);

void main()
{
    vec3 material = texture(materialTex, uv).xyz;
    vec4 objcolor = texture(frameTex, uv);
    vec4 refColor = useReflections? vec4(getSSR(uv, material.r), 1) : vec4(0);
    vec4 bloomColor = useBloom? texture(bloomTex, uv) : vec4(0);

    vec4 indL = useIndirectLight? texture(indirectLight, uv) : vec4(0);
    vec3 c = (objcolor + refColor + bloomColor + indL).xyz; 
    c = c/(c + vec3(1.0));
    c = pow(c, vec3(1.0/2.0));
    

    vec4 part = texture(partTex, uv);
    if (abs(part.w-0.5)<0.01) c = part.xyz;
    color = vec4(c, 1.0);
}

vec3 getSSR(in vec2 uv, float roughness){
    vec3 F = texture(FTex, uv).rgb;
    
    vec2 size = textureSize(reflectTex, 0);
    float dist = textureLod(reflectTex, uv, 0).a * MAX_SSR_DIST;
    float a = roughness * roughness;
    float coneAngle = atan(a * sqrt(1.f/7.f));
    float coneD = 2 * dist * tan(coneAngle/2);

    float mip = clamp(log2(coneD * max(size.x, size.y)), 0, MAX_SSR_LOD);
    return F * mix(texture(IBLReflectTex, uv).rgb, textureLod(reflectTex, uv, mip).rgb, textureLod(reflectMask, uv, mip).r);
}