#version 330 core
#define NEAR 0.1
#define FAR 1000.0
in vec2 UV;
in vec3 viewRay;
out vec4 color;

uniform sampler2D diftex;
uniform sampler2D normtex;
uniform sampler2D depthtex;
const int samples = 32;
uniform float samplesx[samples];
uniform float samplesy[samples];
uniform float samplesz[samples];
uniform vec2 texCoordScales;
uniform mat4 projection;


void main() 
{   
    vec4 c = texture(normtex, UV);
    vec4 d= texture(diftex,UV);
    vec4 e = texture(depthtex,UV);
    if(e.x >= 0.999f){
        color = vec4(0.f, 0.f, 0.f, 1.f);
        return;
    }

    vec3 pos = d.xyz;
    float depth =pos.z;
    vec3 normal = c.xyz*2.0-1.0;
    vec3 rvec = vec3(1.0,1.0,1.0);
    vec3 tangent = normalize(rvec-normal*dot(rvec, normal));
    vec3 bitangent = cross(tangent, normal);
    mat3 rotate = mat3(tangent, bitangent, normal);
    
    float acc = 0.0;
    float radius=0.5;
    for (int i=0; i<samples; i++) 
    {
        vec3 samplePos = rotate*vec3(samplesx[i],samplesy[i],samplesz[i]); //поворачиваем выборку
        samplePos = samplePos+pos; //ограничиваем заданным радиусом и переносим в рассматриваемую точку

        // так как до сих пор мы работали в пространстве вида, то для нахождения проекции выборки необходимо 
        //умножить ее на матрицу проекции
        vec4 shift = projection*vec4(samplePos, 1.0);
        shift.xy /= shift.w;
        shift.xy = shift.xy*0.5+vec2(0.5);
        float sampleDepth =texture(diftex,shift.xy).z;
        if (sampleDepth-depth>0.0025)
        {
            acc+=1.0/(1.0+pow(depth-sampleDepth,2));
        }
    }
    acc=pow(1-acc/samples,2.0);
    color = vec4(acc,acc,acc,1.0);
}
