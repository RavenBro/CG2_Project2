#version 330 core

in vec3 vwpos;
in vec3 vnorm;
layout (location = 6) out vec4 color;

uniform samplerCube cubemap;
uniform vec3 campos;
void main()
{
  vec4 world_pos = vec4(vwpos,1.0);
  vec3 I = normalize(world_pos.xyz-campos);
  vec3 world_norm = vnorm;
  vec3 R = reflect(I, world_norm);
  color = vec4(texture(cubemap, R).xyz,0.5);
  //color = vec4(1,1,1,0.5);
}