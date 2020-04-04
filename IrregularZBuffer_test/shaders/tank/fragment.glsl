#version 330 core
in vec2 vTexCoords;
in vec3 vNormal;

out vec4 color;
uniform int lol;
uniform sampler2D diffuse_tex;
uniform bool use_diffuse_tex = true;
void main()
{
  color = use_diffuse_tex ? vec4(texture(diffuse_tex, vTexCoords).xyz,0.5): vec4(0.8,0.8,0.8,0.5);
  if (color.r<0.01) color.r = 0.01;
}