#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;

out vec3 Normal;
out vec3 Position;

uniform mat4 projection = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 model= mat4(1);

void main()
{
  Normal = mat3(transpose(inverse(model))) * normal;
  Position = vec3(model * vec4(vertex, 1.0));
  gl_Position = vec4(vertex, 1.0f);
  Position = vec3(model * gl_Position);
  gl_Position = projection*view*model*gl_Position;
}