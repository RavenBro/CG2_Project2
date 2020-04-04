#version 330 core
layout (triangles) in ;
layout (triangle_strip, max_vertices = 3) out;

out vec2 UV;

in vec2 vert_UV[];

void main()
{
  gl_Position = gl_in[0].gl_Position;
  UV = vert_UV[0];
  EmitVertex();
  gl_Position = gl_in[1].gl_Position;
  UV = vert_UV[1];
  EmitVertex();
  gl_Position = gl_in[2].gl_Position;
  UV = vert_UV[2];
  EmitVertex();
  EndPrimitive();
}