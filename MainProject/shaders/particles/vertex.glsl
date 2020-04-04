#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in float elem_size;
layout(location = 2) in vec3 colors;

out vec3 vert_cl;
out float size;

void main()
{
  gl_Position = vec4(vertex, 1.0f);
  vert_cl = colors;
  size = elem_size;
}