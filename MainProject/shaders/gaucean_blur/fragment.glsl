#version 330 core

const int COEF_COUNT = 15;

in vec2 UV;

uniform sampler2D src_tex;
uniform bool isHorizontal = true;
uniform int weights = 15;
uniform float weight[COEF_COUNT];

layout(location = 0) out vec4 comp;

void main()
{
    vec2 texStep = 1.f/textureSize(src_tex, 0);
    vec4 res = texture(src_tex, UV) * weight[0];
    if(isHorizontal){
        for(int i = 1; i < weights; i++){
            res += texture(src_tex, UV + vec2(i*texStep.x, 0))*weight[i];
            res += texture(src_tex, UV + vec2(-i*texStep.x, 0))*weight[i];
        }
    } else {
        for(int i = 1; i < COEF_COUNT; i++){
            res += texture(src_tex, UV + vec2(0, i*texStep.y)) * weight[i];
            res += texture(src_tex, UV + vec2(0, -i*texStep.y)) * weight[i];
        }  
    }
    comp = res;
}