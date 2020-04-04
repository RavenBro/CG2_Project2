#version 330 core

in vec2 uv;

uniform sampler2D frameTex;
uniform sampler2D vertexTex;
uniform sampler2D depthTex;
uniform sampler2D materialTex;
uniform sampler2D normalTex;
uniform samplerCube cubeMapTex;
uniform samplerCube skyBoxTex;
uniform mat4 projection;
uniform mat4 caminv;
uniform mat4 cam;
uniform vec3 campos;
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 refraction;
const vec4 skyColor = vec4(0.f, 0.f, 0.f, 1.f);

bool RayMarch(vec3 pos, vec3 dir, out vec3 res);
//bool AccurateRayMarch(vec3 pos, vec3 dir, out vec3 res);
vec3 GetUVD(vec3 pos);
vec3 BinSearch(vec3 start, vec3 end);
float get_depth(vec2 uv)
{
    vec3 pos = texture(depthTex,uv).xyz;
    return(pos.x);
}
void main()
{   vec4 mat = texture(materialTex, uv);
    float reflection = mat.z;
    float transparency = mat.w;
    vec4 pixColor = texture(frameTex, uv);
    vec3 camDir = texture(vertexTex, uv).xyz;
    vec3 normal = normalize(texture(normalTex, uv).xyz);

    refraction = skyColor;
    color = skyColor;
    
    if(reflection > 0.05f)
    {
        vec3 refDir = normalize(reflect(normalize(camDir), normal));
        
        vec3 P = vec3(uv, get_depth(uv));
        
        vec3 p2 = GetUVD(camDir + refDir);
        vec2 size = textureSize(frameTex, 0);
        vec3 D = normalize(p2 - P)/min(size.x, size.y );
        if(D.z > 0){
            vec3 res;
            if(RayMarch(P, D, res))
            {
                if(res.x > 0 && res.x < 1 && res.y > 0 && res.y < 1 && res.z > 0 && res.z < 1)
                {
                    color = texture(frameTex, res.xy);
                    return;
                }
            }
        } 
        
            vec4 world_pos = caminv*texture(vertexTex, uv);
            vec3 I = normalize(world_pos.xyz-campos);
            vec3 world_norm = normalize(mat3(transpose(cam))*normal);
            vec3 R = reflect(I, world_norm);
            
            if (reflection>0.96)
            {
                color = texture(cubeMapTex, R);
                if (abs(color.w-0.5)>0.01) color = texture(skyBoxTex, R);
            }
            else
            {
                color = texture(skyBoxTex, R);
            }
    }
    else
    {
        color = skyColor;
    }
    if (transparency>0.05)
    {
        float ratio = 1.00 / 1.33;
        vec4 world_pos = caminv*texture(vertexTex, uv);
        vec3 I = normalize(world_pos.xyz-campos);
        vec3 world_norm = normalize(mat3(transpose(cam))*normal);
        vec3 R = refract(I, world_norm, ratio);
        refraction = texture(skyBoxTex, R);
    }
    else
    {
        refraction = skyColor;
    }
        
    
}

const int stepsCount = 800;
const int binSearchCount = 10;
const float rayStep = 1;


bool RayMarch(vec3 pos, vec3 dir, out vec3 res)
{
    vec3 ray = pos;
    vec3 delta = dir * rayStep;
    for(int i = 0; i < stepsCount; i++)
    {
        vec3 rayEnd = ray + delta; 

        float depth = get_depth(rayEnd.xy);
        if(rayEnd.z > depth){
            res = BinSearch(ray, rayEnd);
            //res = rayEnd;
            return true;
        }
        ray = rayEnd; 
        if(abs(ray.z - 1) <= 0.01) break;
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

vec3 BinSearch(vec3 start, vec3 end)
{
    vec3 mid;
    for(int i = 0; i < binSearchCount; i++)
    {
        mid = 0.5 * (start + end);
        float depth = get_depth(mid.xy);
        if(mid.z > depth) end = mid;
        else start = mid;
    }
    return mid;
}
