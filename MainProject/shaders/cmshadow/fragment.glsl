#version 330 core

in vec3 worldVert;

uniform float zfar;
uniform vec3 lightPos;

out vec4 color;

void main() {
    float dist = length(worldVert - lightPos)/(2 * zfar);
    color = vec4(dist, dist*dist, 0, 1);
}