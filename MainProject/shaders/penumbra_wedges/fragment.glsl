#version 330 core
in vec4 color;

out vec4 clr;
uniform bool use_diffuse_tex = true;
void main()
{
  clr = color;
}