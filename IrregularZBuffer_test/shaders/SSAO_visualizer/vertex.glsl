#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

out vec2 UV;

void main()
{
  gl_Position = vec4(vertex, 1.0f);
  UV = texCoords;
}