#version 330 core
in vec2 UV;

uniform sampler2D colorTex;
uniform sampler2D maskTex;

uniform bool isHorizontal = true;
uniform int pass;

layout (location = 0) out vec4 resColor;
layout (location = 1) out vec4 resMask;

void main()
{
    vec2 texInv = 1.f/textureSize(colorTex, pass);
    vec4 color = vec4(0);
    vec4 mask = vec4(0);
    if(isHorizontal){
        float texStep = texInv.x;
        if(textureLod(maskTex, UV, pass).r > 0) {
            color += textureLod(colorTex, UV + vec2(texStep * 3, 0), pass) * 0.002; 
            color += textureLod(colorTex, UV + vec2(texStep * 2, 0), pass) * 0.028; 
            color += textureLod(colorTex, UV + vec2(texStep * 1, 0), pass) * 0.233; 
            color += textureLod(colorTex, UV, pass) * 0.474; 
            color += textureLod(colorTex, UV + vec2(-texStep * 1, 0), pass) * 0.233; 
            color += textureLod(colorTex, UV + vec2(-texStep * 2, 0), pass) * 0.028;  
            color += textureLod(colorTex, UV + vec2(-texStep * 3, 0), pass) * 0.002;
        } else {
            color = textureLod(colorTex, UV, pass);
        }

        mask += textureLod(maskTex, UV + vec2(texStep * 3, 0), pass) * 0.002; 
        mask += textureLod(maskTex, UV + vec2(texStep * 2, 0), pass) * 0.028; 
        mask += textureLod(maskTex, UV + vec2(texStep * 1, 0), pass) * 0.233; 
        mask += textureLod(maskTex, UV, pass) * 0.474; 
        mask += textureLod(maskTex, UV + vec2(-texStep * 1, 0), pass) * 0.233; 
        mask += textureLod(maskTex, UV + vec2(-texStep * 2, 0), pass) * 0.028;  
        mask += textureLod(maskTex, UV + vec2(-texStep * 3, 0), pass) * 0.002;
    } else {
        float texStep = texInv.y;
        if(textureLod(maskTex, UV, pass).r > 0){
            color += textureLod(colorTex, UV + vec2(0, texStep * 3), pass) * 0.002; 
            color += textureLod(colorTex, UV + vec2(0, texStep * 2), pass) * 0.028; 
            color += textureLod(colorTex, UV + vec2(0, texStep * 1), pass) * 0.233; 
            color += textureLod(colorTex, UV, pass) * 0.474; 
            color += textureLod(colorTex, UV + vec2(0, -texStep * 1), pass) * 0.233; 
            color += textureLod(colorTex, UV + vec2(0, -texStep * 2), pass) * 0.028;  
            color += textureLod(colorTex, UV + vec2(0, -texStep * 3), pass) * 0.002;
        } else {
            color = textureLod(colorTex, UV, pass);
        }

        mask += textureLod(maskTex, UV + vec2(0, texStep * 3), pass) * 0.002; 
        mask += textureLod(maskTex, UV + vec2(0, texStep * 2), pass) * 0.028; 
        mask += textureLod(maskTex, UV + vec2(0, texStep * 1), pass) * 0.233; 
        mask += textureLod(maskTex, UV, pass) * 0.474; 
        mask += textureLod(maskTex, UV + vec2(0, -texStep * 1), pass) * 0.233; 
        mask += textureLod(maskTex, UV + vec2(0, -texStep * 2), pass) * 0.028;  
        mask += textureLod(maskTex, UV + vec2(0, -texStep * 3), pass) * 0.002;
    }
    resColor = color;
    resMask = mask;
}