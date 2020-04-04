#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D vsm_tex;
uniform float shift = 0.0;
uniform float scale =1.0;
uniform int type = 0;
void main() 
{   
    vec4 c = texture(vsm_tex, scale*UV+shift);
    /*if(c.r >= 1 || c.r <= 0){
        color = vec4(0);
    } else {
        color = vec4(abs(c.r), abs(c.r), abs(c.r), 1.0);
    }*/
    if (type==1)
    {
        float s = 10*(1-c.r);
        color = vec4(s,s,s, 1);
    }
    else if (type==0)
    {
        color = vec4(c.rgb,1.0);
    }
    
}