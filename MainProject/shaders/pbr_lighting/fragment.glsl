#version 330 core

#define MAX_LIGHT_COUNT 16
#define MAX_SM_LIGHT_COUNT 4
#define MAX_VSM_LIGHT_COUNT 2
#define MAX_CM_LIGHT_COUNT 2
#define MAX_SV_LIGHT_COUNT 4
#define MAX_SOFT_SV_LIGHT_COUNT 2

in vec2 uv;

uniform mat4 view;

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;
uniform sampler2D vertexTex;
uniform sampler2D worldVTex;
uniform sampler2D materialTex;
uniform sampler2D skyboxTex;

uniform bool useAO = false;
uniform sampler2D aoTex;

uniform bool useBloom = false;

struct SimplePointLight {
    vec3 position, color; 
};

struct SMPointLight {
    vec3 position, color, normConeDir;
    mat4 viewProj;
    float cosConeAngle;
    sampler2DShadow shadowMap;
};

struct VSMLight {
    vec3 position, color, normConeDir;
    mat4 viewProj;
    float cosConeAngle;
    sampler2D shadowMap;
};

struct CMPointLight {
    float zfar;
    vec3 color, position;
    samplerCube cubemap;
};

struct SVLight
{
    vec3 position, color;
};
struct SoftSVLight
{
    vec3 position, color;
};

layout (std140) uniform SimplePointLightsBlock {
    SimplePointLight spLights[MAX_LIGHT_COUNT];
};
uniform int spLightsCount = 0;

uniform SMPointLight smLights[MAX_SM_LIGHT_COUNT];
uniform int smLightsCount = 0;

uniform VSMLight vsmLights[MAX_VSM_LIGHT_COUNT];
uniform int vsmLightsCount = 0;

uniform CMPointLight cmLights[MAX_CM_LIGHT_COUNT];
uniform int cmLightsCount = 0;

uniform mat4 InverseCamM;

const float SPEC_MIP_COUNT = 5;

uniform vec3 cameraPos;

uniform bool useIBL = false;
uniform samplerCube irradianceMap;
uniform samplerCube specMap;
uniform sampler2D brdfLuT;

uniform sampler2D SVShadeTex;
uniform sampler2D SVSoftShadeTex;  

uniform SVLight svLights[MAX_SV_LIGHT_COUNT];
uniform int svLightsCount = 0;

uniform SoftSVLight ssvLights[MAX_SOFT_SV_LIGHT_COUNT];
uniform int ssvLightsCount = 0;

uniform sampler2D particlesTex;
struct CascadedSMLight
{
    
    mat4 viewProj[3];
    sampler2DShadow Cascade[3];
    float dist[3];
};
uniform CascadedSMLight csmLight;
uniform int vsm_connected = -1;//каскадные тени от некоторых объектов должны сочетаться с vsm тенями от
                          //того же источника

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 spec;
layout (location = 2) out vec4 F;
layout (location = 3) out vec4 bloom;

const float PI = 3.1415927410125732421875f;

vec3 calcF0(vec3 albedo, float metallic)
{
    return mix(vec3(0.04), albedo, metallic);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
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

vec3 LightLo(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 albedo, vec3 radiance, float metallic, float roughness, float ao)
{
    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);        
    float G  = GeometrySmith(N, V, L, roughness);      
    vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0); 

    vec3 kS = F, kD = (1.0 - F) * (1.0 - metallic);
    float NL = max(0, dot(N, L));
    float NV = max(0, dot(N, V));
    vec3 specular = (NDF * G * F)/max((4 * NL * NV), 0.001);
    return (ao * kD * albedo/PI + kS * specular) * radiance * NL;
}

const float err = 0.0001;

float ShadeVSM(vec3 worldVert, in sampler2D vsmTex, in mat4 lightM)
{
    vec4 v =  lightM * vec4(worldVert, 1.0);
    vec3 ndc = v.xyz / v.w;
    ndc = 0.5*ndc + 0.5;
    if(ndc.z <= err || ndc.z > 1.f || ndc.x < 0 || ndc.x > 1.f || ndc.y < 0 || ndc.y >= 1.f - err) return 1.f;

    vec2 vsm = texture(vsmTex, ndc.xy).rg;
    float mu = vsm.x;
    float s2 = vsm.y - mu*mu;
    return s2 / (s2 + (ndc.z - mu)*(ndc.z - mu));
}

vec2 poissonDisk[32] = vec2[](
    vec2(-0.613392, 0.617481),
    vec2(0.170019, -0.040254),
    vec2(-0.299417, 0.791925),
    vec2(0.645680, 0.493210),
    vec2(-0.651784, 0.717887),
    vec2(0.421003, 0.027070),
    vec2(-0.817194, -0.271096),
    vec2(-0.705374, -0.668203),
    vec2(0.977050, -0.108615),
    vec2(0.063326, 0.142369),
    vec2(0.203528, 0.214331),
    vec2(-0.667531, 0.326090),
    vec2(-0.098422, -0.295755),
    vec2(-0.885922, 0.215369),
    vec2(0.566637, 0.605213),
    vec2(0.039766, -0.396100),
    vec2(0.751946, 0.453352),
    vec2(0.078707, -0.715323),
    vec2(-0.075838, -0.529344),
    vec2(0.724479, -0.580798),
    vec2(0.222999, -0.215125),
    vec2(-0.467574, -0.405438),
    vec2(-0.248268, -0.814753),
    vec2(0.354411, -0.887570),
    vec2(0.175817, 0.382366),
    vec2(0.487472, -0.063082),
    vec2(-0.084078, 0.898312),
    vec2(0.488876, -0.783441),
    vec2(0.470016, 0.217933),
    vec2(-0.696890, -0.549791),
    vec2(-0.149693, 0.605762),
    vec2(0.034211, 0.979980)
);

float random(vec4 seed){
    float rnd = dot(seed, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(rnd)*43758.5453);
}

float ShadeSM(vec3 wordVert, in sampler2DShadow shadowM, in mat4 lightM, float bias){
    vec4 lightV = lightM * vec4(wordVert, 1.0);
    vec3 ndc = lightV.xyz/lightV.w;
    ndc = ndc * 0.5 + 0.5;
    if(ndc.z <= err || ndc.z > 1.f || ndc.x < 0 || ndc.x > 1.f || ndc.y < 0 || ndc.y >= 1.f - err) return 1.f;
    ndc.z -= bias;
    float shade = 0.f;
    const int samples = 8;

    for(int i = 0; i < samples; i++){
        int index = int(random(vec4(gl_FragCoord.xyy, i)) * 32) % 32;
        shade += texture(shadowM, ndc + vec3(poissonDisk[index], 0)/256);
    }
    shade /= samples;
    return shade;
}

float ShadePCF(vec3 uv, in sampler2DShadow shadowMap, float bias){
    float shadow = 0.0;
    uv.z -= bias;
    vec2 texelSize = 1.f/textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            shadow += texture(shadowMap, uv);
        }
    }
    return shadow/9.0;
}

const vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

float ShadeCM(vec3 dir, float depth, in samplerCube cubemap){
    vec2 vsm = texture(cubemap, dir).rg;
    float mu = vsm.x;
    float s2 = vsm.y - mu*mu;
    return s2 / (s2 + (depth - mu)*(depth - mu));
}
float ShadeCascade(vec3 vert_cam,vec3 world_pos)
{
    vec4 wp = vec4(world_pos,1.0);
    for (int i=0;i<3;i++)
    {
        if (vert_cam.z>csmLight.dist[i])
        {
            vec4 light_pos = csmLight.viewProj[i]*wp;
            vec3 ndc = light_pos.xyz / light_pos.w;
            
            ndc = 0.5*ndc + 0.5;
            if(ndc.z <= err || ndc.z > 1.f || ndc.x < 0 || ndc.x > 1.f || ndc.y < 0 || ndc.y >= 1.f - err)
                return 1.0;
            /*vec3 vsm;
            if (i==0)
                vsm = texture(csmLight.Cascade[0], ndc.xy).rgb;
            else if (i==1)
                vsm = texture(csmLight.Cascade[1], ndc.xy).rgb;
            else
                vsm = texture(csmLight.Cascade[2], ndc.xy).rgb;
            float mu = vsm.x;
            return (ndc.z-err > mu) ? 0.0 : 1.0;*/
            if (i==0)
                return ShadePCF(ndc, csmLight.Cascade[0], 0.001);
            else if (i==1)
                return ShadePCF(ndc, csmLight.Cascade[1], 0.001);
            else
                return ShadePCF(ndc, csmLight.Cascade[2], 0.001);
        }
    }
    return 1.0;
}
void main()
{   
    vec3 N = vec3(texture(normalTex, uv));
    if(length(N) == 0.0){
        color = vec4(texture(skyboxTex, uv).rgb, 1.0);
        if (useBloom)
        {
            vec4 part = texture(particlesTex,uv);
            if (length(part.xyz)<0.05) bloom = vec4(0,0,0,1);
            else if (abs(part.w-0.1)<0.01) bloom = vec4(25*normalize(part.xyz),1.0);
        }
        else
        {
            bloom = vec4(0, 0, 0, 1);
        }
        F = vec4(0);
        spec = vec4(0);
        return;
    }

    N = normalize(N);
    vec3 pos = vec3(texture(vertexTex, uv));
    vec3 worldPos = (InverseCamM * vec4(pos, 1)).xyz;// texture(worldVTex, uv).xyz;//(inverse(view) * vec4(pos, 1)).xyz;
    vec3 V = normalize(cameraPos - worldPos);

    vec4 material = texture(materialTex, uv);
    float roughness = material.r;
    float metallic = material.g;    
    vec3 albedo = vec3(texture(diffuseTex, uv));
    float ao = useAO? texture(aoTex, uv).r : 1.f;
    vec3 F0 = calcF0(albedo, metallic);
    vec3 Lo = vec3(0);
    
    for(int i = 0; i < spLightsCount; i++)
    {
        vec3 LightPosCam = spLights[i].position;
        vec3 L = normalize(LightPosCam - worldPos);
        float distance    = length(LightPosCam - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = spLights[i].color * attenuation;      
        Lo += LightLo(L, V, N, F0, albedo, radiance, metallic, roughness, ao);
    }
    
    for(int i = 0; i < smLightsCount; i++){
        vec3 LightPosCam = smLights[i].position;
        vec3 L = normalize(LightPosCam - worldPos);
        float bias = 0.f;//max(0.001 * (1 - dot(N, L)), 0.001);
        float shade = ShadeSM(worldPos, smLights[i].shadowMap, smLights[i].viewProj, bias);
        
        float inCone = 0.f;
        float cosLightAngle = dot(normalize(smLights[i].position - worldPos), smLights[i].normConeDir);
        if(cosLightAngle > smLights[i].cosConeAngle){
            inCone = 1.f;
        }

        float distance = length(LightPosCam - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = smLights[i].color * attenuation;     
        Lo += LightLo(L, V, N, F0, albedo, inCone * shade * radiance, metallic, roughness, ao);
    }
    
    for(int i = 0; i < vsmLightsCount; i++){
        float shade = 1.0;
        if (i==vsm_connected)
        {
            shade = ShadeCascade(pos,worldPos);
        }
        else
        {
            shade = ShadeVSM(worldPos, vsmLights[i].shadowMap, vsmLights[i].viewProj);
        }
        vec3 LightPosCam = vsmLights[i].position;
        vec3 L = normalize(LightPosCam - worldPos);
        
        float inCone = 0.f;
        float cosLightAngle = dot(normalize(vsmLights[i].position - worldPos), vsmLights[i].normConeDir);
        if(cosLightAngle > vsmLights[i].cosConeAngle){
            inCone = 1.f;
        }

        float distance = length(LightPosCam - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = vsmLights[i].color * attenuation;     
        Lo += LightLo(L, V, N, F0, albedo, inCone * shade * radiance, metallic, roughness, ao);
    }
    
    for(int i = 0; i < cmLightsCount; i++){
        vec3 LightPosCam = cmLights[i].position;
        vec3 L = normalize(LightPosCam - worldPos);
        float dist = length(cmLights[i].position - worldPos)/(2*cmLights[i].zfar);
        vec3 worldDir = worldPos - cmLights[i].position;
        float shade = ShadeCM(worldDir, dist, cmLights[i].cubemap);
    
        float distance = length(LightPosCam - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = cmLights[i].color * attenuation;     
        Lo += LightLo(L, V, N, F0, albedo, shade * radiance, metallic, roughness, ao);
    }

    for(int i = 0; i < svLightsCount; i++)
    {
        vec3 LightPosCam = svLights[i].position;
        vec3 L = normalize(LightPosCam - worldPos);
        float distance    = length(LightPosCam - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = svLights[i].color * attenuation;
        float shade = 1-texture(SVShadeTex,uv).g;      
        Lo += LightLo(L, V, N, F0, albedo, shade*radiance, metallic, roughness, ao);
    }

    for(int i = 0; i < ssvLightsCount; i++)
    {
        vec3 LightPosCam = ssvLights[i].position;
        vec3 L = normalize(LightPosCam - worldPos);
        float distance    = length(LightPosCam - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = ssvLights[i].color * attenuation;  
        float shade = 1-texture(SVSoftShadeTex,uv).r;     
        Lo += LightLo(L, V, N, F0, albedo, shade*radiance, metallic, roughness, ao);
    }

    vec3 ambient;

    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec2 brdf = texture(brdfLuT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    F = vec4(kS * brdf.x + brdf.y, 0);
    
    if(useIBL){
        vec3 kD = (1.0 - kS) * (1 - metallic);
        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse = irradiance * albedo;
        vec3 R = reflect(-V, N);
        spec = textureLod(specMap, R, roughness * (SPEC_MIP_COUNT - 1)) * ao;
        ambient = (kD * diffuse) * ao; 
    } else {
        ambient = vec3(0.03) * albedo * ao;
        spec = vec4(0);
    }
    
    vec3 c = ambient + Lo;
    c = clamp(c, vec3(0), vec3(5));
    color = vec4(c, 1.f);
    if(useBloom){
        float brightness = dot(c, vec3(0.2126, 0.7152, 0.0722));
        bloom = (brightness > 3.f) ? vec4(c, 1.f) : vec4(0, 0, 0, 1);
        vec4 part = texture(particlesTex,uv);
            if (abs(part.w-0.1)<0.01) bloom = vec4(25*normalize(part.xyz),1.0);
    } else {
        bloom = vec4(0, 0, 0, 1);
    }
}