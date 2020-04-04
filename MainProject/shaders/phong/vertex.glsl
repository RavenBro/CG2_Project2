#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;


out vec2 vTexCoords;
out vec3 vNormal;
out vec3 vFragPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 normMat = mat4(1);

void main()
{
  vTexCoords = texCoords;
  vNormal = vec3(normMat * vec4(normal, 0));
  vFragPos = (view * model*vec4(vertex, 1.0f)).xyz;

  gl_Position = projection * view * model * vec4(vertex, 1.0f);
}