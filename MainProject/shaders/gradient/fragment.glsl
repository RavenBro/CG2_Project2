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

float soft_step_s,soft_step_l;
bool fnd;

float find_average(vec2 UV)
{
    int iter = 7;
    float aver = 0.0;
    vec2 t1 = vec2(0.002,-0.0005);
    vec2 t2 = vec2(-0.002,0.0005);
    vec2 t3 = vec2(-0.0005,0.002);
    vec2 t4 = vec2(0.0005,-0.002);
    vec4 tmp;
    for (int i=0;i<iter;i++)
    {
        tmp = texture(shade,UV+t1);
        aver+=tmp.g*tmp.r;
        tmp = texture(shade,UV+t2);
        aver+=tmp.g*tmp.r;
        tmp = texture(shade,UV+t3);
        aver+=tmp.g*tmp.r;
        tmp = texture(shade,UV+t4);
        aver+=tmp.g*tmp.r;
        t1 = t1*2;
        t2 = t2*2;
        t3 = t3*2;
        t4 = t4*2;
    }
    return(aver/(4.0*iter));

}
float find(vec2 UV, vec2 dir, float dest_color)
{
    int iter = 10;
    dir = normalize(dir)/1000;
    float c;
    for (int i=0;i<iter;i++)
    {
        c = texture(shade,UV+dir).r;
        if (abs(c-dest_color)<0.01) break;
        dir= 2*dir;
    }
    
    if (abs(c-dest_color)>0.01) 
    {
        soft_step_s = 10*max(clamp(max(0.1-dir.x,dir.x-0.9),0,0.1),clamp(max(0.1-dir.y,dir.y-0.9),0,0.1));
        fnd  = false;
        return(1000.0-950*dest_color);
    }
    else
    {
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
        dir = 0.5*dir+0.5*prev_dir;
        vec2 fp = UV+dir;
        soft_step_s = 10*max(clamp(max(0.1-fp.x,fp.x-0.9),0,0.1),clamp(max(0.1-fp.y,fp.y-0.9),0,0.1));
        return(length(dir));
    }
    
}
float random(vec4 seed){
    float rnd = dot(seed, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(rnd)*43758.5453);
}
float find_everywhere(vec2 UV)
{
    vec2 dirs[8] = vec2[]
    (vec2(1,0),
     vec2(0.7071,0.7071),
     vec2(0,1),
     vec2(-0.7071,0.7071), 
     vec2(-1,0),
     vec2(0.7071,-0.7071), 
     vec2(0,-1),
     vec2(-0.7071,-0.7071)
    );

    
    /*vec2 d_dir,l_dir;
    int d_find = 0,l_find = 0;
     bool d_find2 = false,l_find2 = false;
    int iter = 8;
    float cl=0.01;
    float t;
    for (int i=0;i<iter;i++)
    {
        d_dir = vec2(0,0);
        l_dir = vec2(0,0);
        for (int j=0;j<8;j++)
        {
            t=texture(shade,UV+cl*dirs[j]).r;
            if ((!d_find2)&&(t>0.95))
            {
                d_dir += dirs[j];
                d_find++;
            }
            else if ((!l_find2)&&(t<0.05))
            {
                l_dir +=dirs[j];
                l_find++;
            }
        }
        if (d_find>0) 
        {
            d_dir =d_dir/(d_find+0.0);
            d_find2 = true;
        }
        if (l_find>0) 
        {
            l_dir =l_dir/l_find;
            l_find2 = true;
        }
        if (d_find2&&l_find2) break;
        cl=2*cl;
    }
    vec2 prev_dir = d_dir/2;
    vec2 cur_dir;
    for (int i=0;i<iter;i++)
    {
        cur_dir = 0.5*d_dir+0.5*prev_dir;
        t = texture(shade,UV+cur_dir).r;
        if (t>0.9)
        {
            d_dir = cur_dir;
        }
        else
        {
            prev_dir = cur_dir;
        }
    }
    d_dir = 0.5*d_dir+0.5*prev_dir;

    prev_dir = l_dir/2;
    for (int i=0;i<iter;i++)
    {
        cur_dir = 0.5*l_dir+0.5*prev_dir;
        t = texture(shade,UV+cur_dir).r;
        if (t<0.05)
        {
            l_dir = cur_dir;
        }
        else
        {
            prev_dir = cur_dir;
        }
    }
    l_dir = 0.5*l_dir+0.5*prev_dir;
    if (!(d_find2&&l_find2)) fnd=false;
    float d = d_find2?length(d_dir):10.0;
    float f = l_find2?length(l_dir):1.0;*/

    float d = 10.0,f = 10.0;
    vec2 rnd = 0.2*vec2(random(vec4(UV.x,UV.y,UV.y,UV.x)),random(vec4(UV.x,UV.y,UV.x,UV.y)));
    for (int i=0;i<8;i++)
    {
        d = min(d,find(UV,dirs[i]+rnd,1.0));
        f = min(f,find(UV,dirs[i]-rnd,0.0));
    }
    float res = clamp((f-d)/(f+d),-1.0,1.0);
    res = sin(1.5707*res);
    return(0.5*res+0.5);
}
float find2(vec2 UV,vec2 dir)
{
    vec2 dirs[8] = vec2[]
    (vec2(1,0),
     vec2(0.7071,0.7071),
     vec2(0,1),
     vec2(-0.7071,0.7071), 
     vec2(-1,0),
     vec2(0.7071,-0.7071), 
     vec2(0,-1),
     vec2(-0.7071,-0.7071)
    );
    dir = normalize(dir);
    float d = 10.0,f = 10.0;
    vec2 rnd = 0.15*vec2(random(vec4(UV.x,UV.y,UV.y,UV.x)),random(vec4(UV.x,UV.y,UV.x,UV.y)));
    for (int i=0;i<8;i++)
    {
        d = min(d,find(UV,dir+0.5*dirs[i]+rnd,1.0));
        f = min(f,find(UV,-dir+0.5*dirs[i]-rnd,0.0));
    }
    float res = clamp((f-d)/(f+d),-1.0,1.0);
    res = sin(1.5707*res);
    return(0.5*res+0.5);
}
void main() 
{   
    color = vec4(0,0,0,1);
    vec4 base_s = texture(shade,UV);
    float base_cl=base_s.r;
    if (base_cl<0.01) 
        return;
    if (base_cl>0.99) 
    {
        base_cl = base_s.g;
        color = vec4(base_cl,base_cl,base_cl,1.0);
        //color = vec4(1.0);
        return;
    }
    else
    {
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
        fnd = true;
        float d = find(UV,dir,1.0);
        float s = 1- soft_step_s;
        float f = find(UV,-dir,0.0);
        float res = clamp((f-d)/(f+d),-1.0,1.0);
        res = sin(1.5707*res);
        res = 0.5*res+0.5;
        //res =  res*(base_s.g);
        //if ((soft_step_s>0.05)&&(base_s.r<0.9)) res = soft_step_s;
        //soft_step_s = 10*max(clamp(max(0.1-UV.x,UV.x-0.9),0,0.1),clamp(max(0.1-UV.y,UV.y-0.9),0,0.1));
        //if (soft_step_s>0.05)
        //res = mix(find_average(UV),res,soft_step_s);
        //res = find_average(UV);
        //res = base_s.r*base_s.g;
        //res = soft_step_s;
        
        //res = fnd?res:find_everywhere(UV);
        //res = find_everywhere(UV);
        //res = mix(find_everywhere(UV),res,s);
        //res = find2(UV,dir);
        //res =  res*(base_s.g);
        res = find_everywhere(UV);
        soft_step_s = 10*max(clamp(max(0.1-UV.x,UV.x-0.9),0,0.1),clamp(max(0.1-UV.y,UV.y-0.9),0,0.1));
        if (soft_step_s>0.0005)
        {
            res = mix(res,find_average(UV),soft_step_s);
        }
        res = res;
        color = vec4(res,res,res,1);
    }
    
}