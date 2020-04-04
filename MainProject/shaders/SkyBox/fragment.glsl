#version 330 core

in vec3 TexCoords;

uniform samplerCube skybox;
out vec4 color;

void main()
{    
    vec3 c = textureLod(skybox, TexCoords, 1).rgb;
    c = pow(c, vec3(2.2));
    color = vec4(c.x, c.y, c.z, 1);
}