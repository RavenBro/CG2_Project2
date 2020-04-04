#version 330 core

#define LIGHT_COUNT 4

in vec2 uv;

uniform mat4 view;

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;
uniform sampler2D vertexTex;
uniform sampler2D materialTex;

uniform vec3 LightColor[LIGHT_COUNT] = vec3[] (vec3(0), vec3(0), vec3(0), vec3(0));
uniform vec3 LightPos[LIGHT_COUNT] = vec3[] (vec3(0), vec3(0), vec3(0), vec3(0));

layout (location = 0) out vec4 color;


const float PI = 3.1415927410125732421875f;

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

void main()
{   vec3 albedo = vec3(texture(diffuseTex, uv));
    vec3 N = vec3(texture(normalTex, uv));
    N = normalize(N);
    if(length(N) == 0.0){
        color = vec4(albedo, 1.f);
        return;
    }

    
    vec3 pos = vec3(texture(vertexTex, uv));
    vec3 V = normalize(-pos);

    vec4 material = texture(materialTex, uv);
    float roughness = material.r;
    float metallic = material.g;    
    
    if(length(material.xyz) < 0.05)
    {
        color = vec4(albedo, 1.f);
        return;
    }

    vec3 F0 = calcF0(albedo, metallic);
    vec3 Lo = vec3(0);
    for(int i = 0; i < LIGHT_COUNT; i++)
    {
        vec3 LightPosCam = vec3(view * vec4(LightPos[i].xyz, 1.f));
        vec3 L = normalize(LightPosCam - pos);
        vec3 H = normalize(V + L);

        float distance    = length(LightPosCam - pos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = LightColor[i] * attenuation;        
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);        
        float G  = GeometrySmith(N, V, L, roughness);      
        vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);  
            
        // прибавляем результат к исходящей энергетической яркости Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    vec3 ambient = vec3(0.03) * albedo;
    vec3 c = ambient + Lo;
    c = c/(c + vec3(1.0));
    c = pow(c, vec3(1.0/2.0));

    color = vec4(c, 1.f);
}