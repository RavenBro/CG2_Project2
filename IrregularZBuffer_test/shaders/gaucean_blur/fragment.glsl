#version 330 core

const int COEF_COUNT = 5;

in vec2 UV;

uniform sampler2D src_tex;
uniform int isHorizontal;
uniform float weight[COEF_COUNT];
uniform float extra_blur=1.0;
layout(location = 0) out vec4 comp;

void main()
{
    vec2 texStep = 1.f/textureSize(src_tex, 0);

    vec4 base = texture(src_tex, UV) ;
    vec4 res=base* weight[0];
    if(isHorizontal==1)
    {
        float grad=0,delta=0;
        vec4 left,right,dg;
        vec3 aver=vec3(0.0);
        for(int i = 1; i < COEF_COUNT; i++)
        {
            left=texture(src_tex, UV + vec2(-i*texStep.x,0));
            right=texture(src_tex, UV + vec2( i*texStep.x,0));
            aver+=left.xyz;
            aver+=right.xyz;
            grad +=length(left-right);
            res += left* weight[i];
            res += right* weight[i];
        }  
        aver=aver/(2*(COEF_COUNT-1));
        grad=grad/(COEF_COUNT-1);
        delta=length(base.xyz - aver);
        float coef = clamp(delta/(grad+0.001)-1,0,extra_blur);
        res=(1-coef)*res+coef*vec4(aver,1.0);
    }
    else
    {
        float grad=0,delta=0;
        vec4 left,right,dg;
        vec3 aver=vec3(0.0);
        for(int i = 1; i < COEF_COUNT; i++)
        {
            left=texture(src_tex, UV + vec2(0, -i*texStep.y));
            right=texture(src_tex, UV + vec2(0, i*texStep.y));
            aver+=left.xyz;
            aver+=right.xyz;
            grad +=length(left-right)/(2*i);
            res += left* weight[i];
            res += right* weight[i];
        }  
        aver=aver/(2*(COEF_COUNT-1));
        grad=grad/(COEF_COUNT-1);
        delta=length(base.xyz - aver);
        float coef = clamp(delta/(grad+0.001)-1,0,extra_blur);
        res=(1-coef)*res+coef*vec4(aver,1.0);
    } /*else {
        for(int i = 1; i < COEF_COUNT; i++){
            res += texture(src_tex, UV + vec2(0, i*texStep.y)) * weight[i];
            res += texture(src_tex, UV + vec2(0, -i*texStep.y)) * weight[i];
        }  
    }*/
    comp = res;
    //comp = vec4(1.0,0.0,0.0,1.0);
}