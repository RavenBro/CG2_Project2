#version 330 core


uniform sampler2D frameTex;
uniform sampler2D reflectTex; 
uniform sampler2D materialTex;
uniform sampler2D shadeTex;
uniform sampler2D bloomTex;
uniform sampler2D SSAOTex;
uniform sampler2D diffuseTex;
uniform sampler2D refractTex;
uniform float cho_to_temnovato = 1.25;
uniform float ambient = 0.01;
uniform int df,dr,dm,dsh,db,dss,dd;
in vec2 uv;

out vec4 color;

void main()
{
    vec4 objcolor = df*texture(frameTex, uv);
    vec4 refColor = dr*texture(reflectTex, uv);
    vec4 refractColor = dr*texture(refractTex, uv);
    vec4 diffuseColor = dd*texture(diffuseTex,uv);
    objcolor = mix(objcolor,diffuseColor,dd*ambient);
    vec4 mat = texture(materialTex, uv);
    if(mat.w>0.99) mat.w=0;
    float refc = mat.z;
    float transc=1-mat.w;
    color = mix(objcolor, refColor, dr*refc); 
    color = mix(color, refractColor, dr*mat.w);
    vec4 ao = dss*texture(SSAOTex, uv);
    color = color+0.001*dss*transc*0.6*(ao-vec4(1.0,1.0,1.0,0.0));
    color = color + db*0.75*texture(bloomTex, uv);
    //color = color - dsh*transc*0.7*texture(shadeTex, uv);
    color = color * (1-0.7*dsh*texture(shadeTex, uv));
    color = mix(mat,color,dm);
    color.xyz *=cho_to_temnovato;
}