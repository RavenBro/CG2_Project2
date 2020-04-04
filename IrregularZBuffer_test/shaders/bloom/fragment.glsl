#version 330 core

in vec2 UV;
out vec4 color;
uniform sampler2D tex;

void main() 
{   
    vec4 bc = texture(tex, UV);
    float brightness = dot(bc.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.55)
        color = vec4(bc.rgb, 1.0);
    else
        color = vec4(0.0, 0.0, 0.0, 1.0);
}