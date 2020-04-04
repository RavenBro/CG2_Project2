#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
out vec3 vwpos;
out vec3 vnorm;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
  gl_Position = model*vec4(vertex, 1.0f);
  vwpos = gl_Position.xyz;
  vnorm = mat3(transpose(inverse(model)))*normal;
  gl_Position = projection*view*model*vec4(vertex, 1.0f);
}