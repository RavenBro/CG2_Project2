#version 330 core
layout(location = 0) in vec3 vertex;

uniform mat4 projection = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 model = mat4(1);

out vec3 vWorldPos;
void main()
{
  gl_Position = model*vec4(vertex, 1.0f);
  vWorldPos = gl_Position.xyz;
  gl_Position = projection*view*gl_Position;

  
}