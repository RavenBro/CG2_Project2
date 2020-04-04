#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D vsm_tex;

void main() 
{   
    vec4 c = textureLod(vsm_tex, UV, 3);
    /*if(c.r >= 1 || c.r <= 0){
        color = vec4(0);
    } else {
        color = vec4(abs(c.r), abs(c.r), abs(c.r), 1.0);
    }*/
    color = vec4(c.rgb, 1);
}