#version 450 core
#define W 800
#define H 600
in vec2 UV;

out vec4 color;

uniform mat4 L_Vinv;
uniform sampler2D vertex_tex;
layout(std430, binding = 3) buffer light_depth
{
    readonly uint depth[W*H];
};
void main() 
{   color = vec4(0,0,0, 1.0);
    uvec2 pixUV = uvec2(uint(trunc(UV.x*W)),uint(trunc(UV.y*H)));
    float d = depth[pixUV.y*W + pixUV.x]/(1000.0*1000*1000);
    vec4 c = vec4(texture(vertex_tex, UV).rgb,1.0);
    if (length(c.xyz)<0.0001) return;
    c = L_Vinv*c;
    vec3 ndc = c.xyz / c.w;
    ndc = 0.5*ndc + 0.5;
    d = ndc.z-d>0.0005?1:0;
    color = vec4(d,d,d, 1.0);
}