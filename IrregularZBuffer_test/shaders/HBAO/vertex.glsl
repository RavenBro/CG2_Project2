#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
out vec2 TexCoord;
out vec2 Position;
void main()
{
  gl_Position = vec4(vertex, 1.0f);
  TexCoord= texCoords;
  Position = texCoords * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
}