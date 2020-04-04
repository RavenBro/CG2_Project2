#version 330 core
layout (points) in ;
layout (triangle_strip, max_vertices = 12) out;

out vec4 cl;

in vec3 vert_cl[];
in float size[];
uniform mat4 VP;
uniform int type;
uniform int is_light;
void main()
{
  float l = 0.1*is_light;
  if (type==0)
  {
    vec4 pos = VP*gl_in[0].gl_Position;
    float sz = size[0];
    gl_Position = pos+vec4(sz,sz,0,0);
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = pos+vec4(-sz,sz,0,0);
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = pos+vec4(sz,-sz,0,0);
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = pos+vec4(-sz,-sz,0,0);
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    EndPrimitive();
  }
  else if (type == 1)
  {
    vec4 pos = gl_in[0].gl_Position;
    float sz = size[0];
    gl_Position = VP*(pos+vec4(0,0,0,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(sz,0,0,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(0,0,sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(0,sz,0,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(0,0,0,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(sz,0,0,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    EndPrimitive();
    /*gl_Position = VP*(pos+vec4(sz,sz,sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(sz,sz,-sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(-sz,sz,-sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(-sz,sz,sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(-sz,-sz,sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(-sz,-sz,-sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(sz,-sz,-sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(sz,-sz,sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(sz,sz,sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    gl_Position = VP*(pos+vec4(sz,sz,-sz,0));
    cl = vec4(vert_cl[0],l);
    EmitVertex();
    EndPrimitive();*/
  }
}