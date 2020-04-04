#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 2) in vec2 texCoords;

out vec2 UV;
uniform float v_shiftx = 0;
uniform float v_scalex =1;
uniform float v_shifty = 0;
uniform float v_scaley =1;
void main()
{
  vec3 res = vec3(v_scalex*vertex.x+v_shiftx,v_scaley*vertex.y+v_shifty,vertex.z);
  gl_Position = vec4(res, 1.0f);
  UV = texCoords;
}