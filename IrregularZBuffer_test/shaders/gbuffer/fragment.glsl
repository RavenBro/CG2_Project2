#version 330 core
#define FAR 100
#define NEAR 0.1
in vec2 uv_ndc;
in vec3 vert_cam;
in vec3 normal;
in vec4 coords_for_shades;
in vec4 world_pos;
const int NUM_CASCADES = 3;
 
in vec4 LightSpacePos[NUM_CASCADES];

uniform float gCascadeEndClipSpace[NUM_CASCADES];

uniform bool use_diffuse_tex = true;
uniform int use_shadows = 1;
uniform float metallic = 0.5f;
uniform float roughness = 0.f;
uniform float reflection = 0.f;
uniform float transparency = 0.f;
uniform sampler2D diffuse_tex;
uniform sampler2D vsm_tex;
uniform sampler2D nearCascade;
uniform sampler2D middleCascade;
uniform sampler2D farCascade;
uniform samplerCube cubeMapShadows;
uniform sampler2DShadow shadowMap;
uniform vec3 pointLightPos;

layout (location = 0) out vec4 diffuse_color;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec3 camera_v;
layout (location = 3) out vec4 shaded_color;
layout (location = 4) out vec4 material;


vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);
float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}
float ShadeCascade()
{
    if (vert_cam.z>gCascadeEndClipSpace[0])
    {
        //shaded_color.r+=0.4;
        vec3 ndc = LightSpacePos[0].xyz / LightSpacePos[0].w;
        ndc = 0.5*ndc + 0.5;
        if(ndc.z > 1.f) return 0.f;
        
        vec3 vsm = texture(nearCascade, ndc.xy).rgb;
        float mu = vsm.x;
        float s2 = vsm.y - mu*mu;
        
        return 0.77*(1-vsm.b)*(1.f - (s2 / (s2 + (ndc.z - mu)*(ndc.z - mu))));
    }
    else if (vert_cam.z>gCascadeEndClipSpace[1])
    {
        //shaded_color.g+=0.4;
        vec3 ndc = LightSpacePos[1].xyz / LightSpacePos[1].w;
        ndc = 0.5*ndc + 0.5;
        if(ndc.z > 1.f) return 0.f;

        vec3 vsm = texture(middleCascade, ndc.xy).rgb;
        float mu = vsm.x;
        float s2 = vsm.y - mu*mu;
        
        return (1-vsm.b)*(1.f - (s2 / (s2 + (ndc.z - mu)*(ndc.z - mu))));
    }
    else
    {
        //shaded_color.b+=0.4;
        vec3 ndc = LightSpacePos[2].xyz / LightSpacePos[2].w;
        ndc = 0.5*ndc + 0.5;
        if(ndc.z > 1.f) return 0.f;

        vec3 vsm = texture(farCascade, ndc.xy).rgb;
        float mu = vsm.x;
        float s2 = vsm.y - mu*mu;
        
        return (1-vsm.b)*(1.f - (s2 / (s2 + (ndc.z - mu)*(ndc.z - mu))));
    }
}
float ShadeVSM(vec4 modelVert)
{
    vec3 ndc = modelVert.xyz / modelVert.w;
    ndc = 0.5*ndc + 0.5;
    if(ndc.z > 1.f) return 0.f;

    vec3 vsm = texture(vsm_tex, ndc.xy).rgb;
    float mu = vsm.x;
    float s2 = vsm.y - mu*mu;
    return (1-vsm.b)*(1.f - (s2 / (s2 + (ndc.z - mu)*(ndc.z - mu))));
}
float ShadeSimple(vec4 modelVert)
{
    vec3 ndc = modelVert.xyz / modelVert.w;
    ndc = 0.5*ndc + 0.5;
    if(ndc.z > 1.f) return 0.f;
    
    float bias = 0.005;
    ndc.z -= bias;
    return 1-texture(shadowMap, ndc);
}
float ShadePoissonDisc(vec4 modelVert)
{   
    vec3 ndc = modelVert.xyz / modelVert.w;
    ndc = 0.5*ndc + 0.5;
    if(ndc.z > 1.f) return 0.f;
    float shade =0.0;
    int samples_n = 4, hits = 0;
    int s_dists[3] = int[](1000,500,250);
    float bias = 0.0005;
    vec3 real_vsm = texture(vsm_tex, ndc.xy).rgb;
    float real_mu = real_vsm.x;
    if (ndc.z-real_mu-bias>0) hits++;
    for (int i=0;i<samples_n;i++)
    {
        int index = int(16.0*random(world_pos.xyz, i))%16;
        vec3 vsm = texture(vsm_tex, ndc.xy+poissonDisk[index]/s_dists[0]).rgb;
        float mu = vsm.x;
        if (ndc.z-mu-bias-length(poissonDisk[index])/s_dists[0]<0) continue;
        hits++;
        float s2 = vsm.y - mu*mu;
        shade +=(1-vsm.b)*clamp((1.f - (s2 / (s2 + (ndc.z - mu)*(ndc.z - mu)))),0.0,1.0);
    }
    if ((hits==5) || (hits==0)) return (shade/samples_n);
    samples_n+=4;
    for (int i=4;i<samples_n;i++)
    {
        int index = int(16.0*random(world_pos.xyz, i))%16;
        vec3 vsm = texture(vsm_tex, ndc.xy+poissonDisk[index]/s_dists[1]).rgb;
        float mu = vsm.x;
        if (ndc.z-mu-bias-length(poissonDisk[index])/s_dists[1]<0) continue;
        hits++;
        float s2 = vsm.y - mu*mu;
        shade +=(1-vsm.b)*clamp((1.f - (s2 / (s2 + (ndc.z - mu)*(ndc.z - mu)))),0.0,1.0);
    }
    if (hits==8) return (shade/samples_n);
    samples_n+=8;
    for (int i=8;i<samples_n;i++)
    {
        int index = int(16.0*random(world_pos.xyz, i))%16;
        vec3 vsm = texture(vsm_tex, ndc.xy+poissonDisk[index]/s_dists[2]).rgb;
        float mu = vsm.x;
        if (ndc.z-mu-bias-length(poissonDisk[index])/s_dists[2]<0) continue;
        hits++;
        float s2 = vsm.y - mu*mu;
        shade +=(1-vsm.b)*clamp((1.f - (s2 / (s2 + (ndc.z - mu)*(ndc.z - mu)))),0.0,1.0);
    }
    return (shade/samples_n);
}
float ShadeCube(vec4 modelVert)
{
    int samples_n = 4, hits = 0;
    int s_dists[3] = int[](30,60,200);
    float pmin = 0.001;
    float pmax=0.05;
    vec2 vsm;
    float delta, shade =0.0;
    
    vec3 fragToLight = modelVert.xyz - pointLightPos; 
    float dep = sqrt(length(fragToLight)/FAR) - pmin;

    vsm = texture(cubeMapShadows, normalize(fragToLight)).rg;
        delta = dep - vsm.x;
        if (delta>0) hits++;
    for (int i=0;i<samples_n;i++)
    {
        vsm = texture(cubeMapShadows, normalize(fragToLight+
                      vec3(poissonDisk[i].x,0.0,poissonDisk[i].y)/s_dists[0])).rg;
        delta = dep - vsm.x - length(poissonDisk[i])/s_dists[0];
        if (delta<0) continue;
        hits++;
        shade +=(1-vsm.y)*((delta>pmax) ? 1-delta+pmax: 1);
    }
    if ((hits==5) || (hits==0)) return (shade/samples_n);
    samples_n+=4;
    for (int i=4;i<samples_n;i++)
    {
        vsm = texture(cubeMapShadows, normalize(fragToLight+
                      vec3(poissonDisk[i].x,0.0,poissonDisk[i].y)/s_dists[1])).rg;
        delta = dep - vsm.x - length(poissonDisk[i])/s_dists[1];
        if (delta<0) continue;
        hits++;
        shade +=(1-vsm.y)*((delta>pmax) ? 1-delta+pmax: 1);
    }
    if (hits==8) return (shade/samples_n);
    samples_n+=8;
    for (int i=8;i<samples_n;i++)
    {
        vsm = texture(cubeMapShadows, normalize(fragToLight+
                      vec3(poissonDisk[i].x,0.0,poissonDisk[i].y)/s_dists[1])).rg;
        delta = dep - vsm.x - length(poissonDisk[i])/s_dists[1];
        if (delta<0) continue;
        hits++;
        shade +=(1-vsm.y)*((delta>pmax) ? 1-delta+pmax: 1);
    }
    return (shade/samples_n);
    /*vsm = texture(cubeMapShadows, normalize(fragToLight)).rg;
    delta = dep - vsm.x;
    if (delta<0) return 0.0;
    shade +=(1-vsm.y)*((delta>pmax) ? 1-delta+pmax: 1);
    return(shade);*/
}
void main()
{
    shaded_color=vec4(0.0,0.0,0.0,1.0);
    diffuse_color = (use_diffuse_tex)? texture(diffuse_tex, uv_ndc) : vec4(0.6f, 0.8f, 1.0f, 1.f);
    camera_v = vert_cam;
    out_normal = normal;
    material = vec4(roughness, metallic, reflection, transparency);
    if (use_shadows == 1)
    {
        float shade;
        //shade = 1.5*ShadePoissonDisc(coords_for_shades);
        shade = ShadeSimple(coords_for_shades);
        shaded_color=vec4(shade,shade,shade,1.0);
    }
    else if (use_shadows == 2)
    {
        float shade;
        shade = 1.5*ShadeVSM(coords_for_shades);
        shaded_color=vec4(shade,shade,shade,1.0);
    }
    else if (use_shadows == 3)
    {
        float shade;
        shade = ShadeCascade();
        shaded_color=vec4(shade,shade,shade,1.0);
    }
    else if (use_shadows == 4)
    {
        float shade;
        shade = ShadeCube(world_pos);
        shaded_color=vec4(shade,shade,shade,1.0);
    }
    else if (use_shadows == 5)
    {
        float shade;
        shade = ShadeSimple(coords_for_shades);
        shaded_color=vec4(shade,shade,shade,1.0);
    }
}