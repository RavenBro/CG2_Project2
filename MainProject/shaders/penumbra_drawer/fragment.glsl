#version 330 core
in vec3 vWorldPos;

out vec4 color;
uniform vec3 light;
uniform float sc=50;
uniform float dist = 0.0;
void main()
{
  //float scale=0.6*clamp(1-(length(light-vWorldPos)-dist)/sc,0.0,1.0);
  float scale = clamp(1-(length(light-vWorldPos)-dist)/sc,0.0,0.95);
  color = vec4(0.5,scale,scale,1.0);
}