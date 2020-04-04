#version 330 core

layout (location = 0) in vec3 modelVert;

uniform mat4 viewProj;
uniform mat4 model;

out vec3 worldVert; 

void main(){
    vec4 temp = model * vec4(modelVert, 1);
    worldVert = temp.xyz;
    gl_Position = viewProj * temp;
}