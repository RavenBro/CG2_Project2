#version 450 core
#define W 800
#define H 600
#define CLUST 32
in vec2 UV;

out vec4 color;
//uniform mat4 proj;
uniform mat4 L_Vinv;
uniform sampler2D vertex_tex;
struct node
{
    uint link;
    uvec2 vp_pos;
    vec2 lp_pos;
};
layout(std430, binding = 2) buffer data
{
    volatile uint counters[CLUST];
    coherent uint head_data[W*H];
    writeonly node nodes[W*H];
};
void main() 
{   color = vec4(0,0,0, 1.0);
    uvec2 pix;
    pix.x = uint(UV.x*W);
    pix.y = uint(UV.y*H);
    uint pos = uint(trunc(pix.x) + trunc(pix.y)*W);
    
    
    vec4 c = vec4(texture(vertex_tex, UV).rgb,1.0);
    if (length(c.xyz)<0.0001) return;
    //c = proj*L_Vinv*c;
    c = L_Vinv*c;
    vec3 ndc = c.xyz / c.w;
    ndc = 0.5*ndc + 0.5;

    
    uvec2 pixl;
    pixl.x = uint(ndc.x*W);
    pixl.y = uint(ndc.y*H);
    uint posl = uint(trunc(pixl.x) + trunc(pixl.y)*W);

    if ((ndc.x<0)||(ndc.x>1)) {ndc =vec3(0); return;}
    if ((ndc.y<0)||(ndc.y>1)) {ndc =vec3(0); return;}

    uint a = atomicAdd(counters[pos%CLUST],CLUST);
    nodes[a].vp_pos = pix;
    nodes[a].lp_pos = ndc.xy;
    nodes[a].link = atomicExchange(head_data[posl],a);
    
    //if ((ndc.z<0)||(ndc.z>1)) ndc =vec3(0);
    posl = posl/4;
    color = vec4(posl%2/2.0,posl%3/3.0,posl%7/7.0, 1.0);
}