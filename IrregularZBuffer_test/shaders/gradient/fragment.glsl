#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D shade;
uniform sampler2D vertex;
//uniform sampler2D normal;

uniform vec3 light_pos;
vec3 center_pos;
uniform vec3 shadow_centers_screen[256];
uniform vec3 shadow_centers_camera[256];
uniform float shadow_sizes[256];
uniform int shadow_centers;
float find(vec2 UV, vec2 dir, float dest_color)
{
    int iter = 8;
    dir = normalize(dir)/400;
    float c;
    for (int i=0;i<iter;i++)
    {
        c = texture(shade,UV+dir).r;
        if (abs(c-dest_color)<0.01) break;
        dir= 2*dir;
    }
    if (abs(c-dest_color)>0.01) return(1000.0);
    vec2 prev_dir = dir/2;
    vec2 cur_dir;
    for (int i=0;i<iter;i++)
    {
        cur_dir = 0.5*dir+0.5*prev_dir;
        c = texture(shade,UV+cur_dir).r;
        if (abs(c-dest_color)<0.01)
        {
            dir = cur_dir;
        }
        else
        {
            prev_dir = cur_dir;
        }
    }
    return(length(0.5*dir+0.5*prev_dir));
}
void main() 
{   
    color = vec4(0,0,0,1);
    vec4 base_s = texture(shade,UV);
    float base_cl=base_s.r;
    if (base_cl<0.01) 
        return;
    /*if (base_cl>0.9) 
    {
        base_cl = base_cl * (1-base_s.g);
        color = vec4(base_cl,base_cl,base_cl,1.0);
        color = vec4(0,1,1,1);
        return;
    }*/
    vec3 pos_camera = texture(vertex,UV).xyz;
    float min_dist = 10000.0;
    int min_n = -1;
    for (int i=0;i<shadow_centers;i++)
    {
        float tmp = length(shadow_centers_camera[i]-pos_camera);
        if ((tmp<min_dist)&&(tmp<shadow_sizes[i]))
        {
            min_n =i;
            min_dist = tmp;
        }
    }
    /*if (min_n==0) color = vec4(1.0,0.0,0.0,1.0);
    else if (min_n==1) color = vec4(0.0,1.0,0.0,1.0);
    else if (min_n==2) color = vec4(0.0,0.0,1.0,1.0);
    else color = vec4(0.0,1.0,1.0,1.0);
    return;*/
    center_pos =  shadow_centers_screen[min_n];
    vec2 dir = normalize(center_pos.xy - UV);
    float d = find(UV,dir,1.0);
    float f = find(UV,-dir,0.0);
    float res = clamp((f-d)/(f+d),-1.0,1.0);
    res = sin(1.5707*res);
    res = 0.5*res+0.5;
    res =  res*(1-base_s.g);
    color = vec4(res,res,res,1.0);
}