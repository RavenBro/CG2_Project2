#version 330 core
layout(location = 0) in vec3 vertex;


uniform mat4 projection = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 model = mat4(1);

void main()
{
  gl_Position = vec4(vertex, 1.0f);
  gl_Position = projection*view*model*gl_Position;
}