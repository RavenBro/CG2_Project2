#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
const float aspect=3.0/4.0;
const float fov=3.141/2.0;

out vec2 UV;
out vec3 viewRay;
void main()
{
  gl_Position = vec4(vertex, 1.0f);
  viewRay = vec3(-vertex.x*aspect*fov, -vertex.y*fov, 1.0);
  UV = texCoords;
  //UV = (vertex.xy + vec2(1.0)) / 2.0;
}