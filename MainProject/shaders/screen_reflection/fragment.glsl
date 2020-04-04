#version 330 core

in vec2 uv;

uniform sampler2D frameTex;
uniform sampler2D vertexTex;
uniform sampler2D depthTex;
uniform sampler2D materialTex;
uniform sampler2D normalTex;
//uniform sampler2D albedoTex;
//uniform sampler2D brdfLuT;

uniform mat4 projection;
uniform mat4 InvView = mat4(1);
uniform mat4 normalMat;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 mask;

const float PI = 3.1415927410125732421875f;
const int MAX_MIPMAP = 5;
const float MAX_DIST = 4.f;
const vec4 skyColor = vec4(0.f, 0.f, 0.f, 1.f);

bool RayMarch(vec3 pos, vec3 dir, out vec3 res, float rayStep);
bool AccurateRayMarch(vec3 sP1, vec3 sP2, float step, out vec3 res);
vec3 GetUVD(vec3 pos);

vec3 calcF0(vec3 albedo, float metallic);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 LightLo(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 albedo, vec3 radiance, float metallic, float roughness);

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

float screenFade(vec2 uv);
const float Scale = 0.5; 
const vec3 K = vec3(19.19);

vec3 hash(vec3 a)
{
    a = fract(a * Scale);
    a += dot(a, a.yxz + K);
    return fract((a.xxy + a.yxx)*a.zyx);
}

void main()
{
    vec4 material = texture(materialTex, uv);
    float roughness = material.r;
    float metallic = material.g;
    float reflection = material.b;

    if(reflection > 0.f)
    {
        vec3 camDir = texture(vertexTex, uv).xyz;
        vec3 normal = (normalMat *  vec4(texture(normalTex, uv).xyz, 0)).xyz;
        vec3 refDir = normalize(reflect(normalize(camDir), normal));

        if(dot(refDir, vec3(0, 0, 1)) > 0){
            mask = vec4(0);
            color = vec4(0);
            return;
        }

        vec2 size = textureSize(frameTex, 0);
        vec3 P = vec3(uv, texture(depthTex, uv).x);
        
        vec3 p2 = GetUVD(camDir + refDir);
        vec3 D = (p2 - P)/(p2 - P).z;
        vec3 res;
        float stepRay = (0.5 + 4 * roughness)/(2 * max(size.x, size.y));
        
        if(RayMarch(P, D, res, stepRay))
        {
            if(res.x > 0 && res.x < 1 && res.y > 0 && res.y < 1 && res.z > 0 && res.z < 1)
            {
                vec3 hit = texture(vertexTex, res.xy).xyz;
                vec3 coneTraced = texture(frameTex, res.xy).rgb;
                float dist = length(hit - camDir);
                float distFade = 1.0f - clamp(dist/2, 0, 1);
                distFade *= distFade;
                color = vec4(distFade * screenFade(res.xy) * coneTraced, dist/MAX_DIST);
                mask = vec4(1.f);
                return;
            }
        } 
    }
    color = skyColor;
    mask = vec4(0);
}



float screenFade(vec2 uv)
{
    const float a = -21.053; 
    const float c = 5.263;

    float shade1, shade2;
    float v1 = abs(uv.x - 0.5); // <= 0.5
    if(v1 < 0.45) shade1 = 1.f;
    shade1 = a * v1 * v1 + c;

    float v2 = abs(uv.y - 0.5); // <= 0.5
    if(v2 < 0.45) shade2 = 1.f;
    shade2 = a * v2 * v2 + c;
    return min(shade1, shade2);
}

const int stepsCount = 800;
const int binSearchCount = 10;

vec3 BinSearch(vec3 start, vec3 delta){
    for(int i = 0; i < binSearchCount; i++){
        delta = 0.5 * delta;
        vec3 end = start + delta;
        if(end.z < texture(depthTex, end.xy).r) return end;
    }
    return (start + delta);
}

bool RayMarch(vec3 pos, vec3 dir, out vec3 res, float stepRay)
{
    vec3 ray = pos;
    vec3 delta = dir * stepRay;
    ray += delta;
    for(int i = 1; i < stepsCount; i++)
    {
        vec3 rayEnd = ray + i * delta; 

        float depth = texture(depthTex, rayEnd.xy).x;
        if(rayEnd.z > depth){
            res = BinSearch(rayEnd - delta, delta);
            return true;
        }
        //if(abs(rayEnd.z - 1) <= 0.00001) break;
    }
    return false;
}

const float maxSteps = 100.f;

bool AccurateRayMarch(vec3 sP1, vec3 sP2, float step, out vec3 res){
    if(length(sP1.xy - sP2.xy) < 0.01f) return false;

    float deltaX = sP2.x - sP1.x, deltaY = sP2.y - sP1.y, deltaD = sP2.z - sP1.z;

    if(abs(deltaX) > abs(deltaY)){
        float d0 = sP1.z;
        float y0 = sP1.y;
        float Cy = deltaY/deltaX, Cd = deltaD/deltaX;
        
        float dir = sign(deltaX);

        for(float i = 0; i <= abs(deltaX) && i < maxSteps; i+= step){
            float x = sP1.x + i * dir;
            float y = y0 + Cy * (x - sP1.x);
            float d = d0 + Cd * (x - sP1.x);
            if(texelFetch(depthTex, ivec2(x, y), 0).r < d){
                res = vec3(x, y, d);
                return true;
            }
        }
    } else {
        float d0 = sP1.z;
        float x0 = sP1.x;
        float Cx = deltaX/deltaY, Cd = deltaD/deltaY;

        float dir = abs(deltaY);
        for(float i = 0; i <= abs(deltaY) && i < maxSteps; i += step){
            float y = sP1.y + i * dir;
            float x = x0 + Cx * (y - sP1.y);
            float d = d0 + Cd * (y - sP1.y);
            if(texelFetch(depthTex, ivec2(x, y), 0).r < d){
                res = vec3(x, y, d);
                return true;
            }
        }

    }
    return false;
}

vec3 GetUVD(vec3 pos)
{
    vec4 proj = projection * vec4(pos, 1.0);
    proj /= proj.w;
    proj.xyz = proj.xyz * 0.5 + 0.5;
    return proj.xyz;
}


vec3 calcF0(vec3 albedo, float metallic)
{
    return mix(vec3(0.04), albedo, metallic);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 LightLo(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 albedo, vec3 radiance, float metallic, float roughness)
{
    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);        
    float G  = GeometrySmith(N, V, L, roughness);      
    vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0); 

    vec3 kS = F, kD = (1.0 - F) * (1.0 - metallic);
    float NL = max(0, dot(N, L));
    float NV = max(0, dot(N, V));
    vec3 specular = (NDF * G * F)/max((4 * NL * NV), 0.001);
    return (kD * albedo/PI + kS * specular) * radiance * NL;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}