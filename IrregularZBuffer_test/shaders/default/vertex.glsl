#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model = mat4(1.f);
uniform mat3 norm = mat3(1.f);

uniform mat4 lightNDC = mat4(1.f);

out vec2 UV;
out vec3 vNormal;
out vec4 vModel;

void main(void){
    vModel = model * vec4(vertex, 1.0);
    gl_Position = projection * view * vModel; 
    vNormal = norm * normal;
    vModel = lightNDC * vModel;
    UV = texCoords;
}