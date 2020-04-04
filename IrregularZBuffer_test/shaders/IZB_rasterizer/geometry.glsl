#version 450 core
#define W 800
#define H 600
#define CLUST 32
layout (triangles) in ;
layout (triangle_strip, max_vertices = 3) out;


uniform mat4 MVP;
struct node
{
    uint link;
    uvec2 vp_pos;
    vec2 lp_pos;
};
layout(std430, binding = 2) buffer data
{
    readonly uint counters[CLUST];
    readonly uint head_data[W*H];
    readonly node nodes[W*H];
};
layout(std430, binding = 3) buffer light_depth
{
    coherent uint depth[W*H];
};
vec3 proj(vec4 a)
{
  a = MVP*a;
  a = a/a.w;
  return vec3(0.5)+0.5*a.xyz;
}
vec4 barycentric(vec2 p, vec2 a, vec2 b, vec2 c)
{
  float s1 = cross(vec3(c-p,0.0),vec3(b-p,0.0)).z;
  if (s1>0)
  {
    float s2 = cross(vec3(a-p,0.0),vec3(c-p,0.0)).z;
    if (s2>0)
    {
      float s3 = cross(vec3(b-p,0.0),vec3(a-p,0.0)).z;
      if (s3>0)
      {
        float s = s1+s2+s3;
        return(vec4(s1/s,s2/s,s3/s,1.0));
      }
    }
  }
  return(vec4(0,0,0,0));
  //bar.x =( (p.y-c.y)*(b.x-c.x)-(p.x-c.x)*(b.y-c.y) )/( (a.y-c.y)*(b.x-c.x) - (a.x-c.x)*(b.y-c.y) );

  //bar.y =( (p.y-a.y)*(b.x-a.x)-(p.x-a.x)*(b.y-a.y) )/( (a.y-c.y)*(b.x-c.x) - (a.x-c.x)*(b.y-c.y) );

  //bar.z =( (p.y-a.y)*(b.x-c.x)-(p.x-c.x)*(b.y-c.y) )/( (a.y-c.y)*(b.x-c.x) - (a.x-c.x)*(b.y-c.y) );
}
void main()
{
  vec3 a,b,c;
  a = proj(gl_in[0].gl_Position);
  b = proj(gl_in[1].gl_Position);
  c = proj(gl_in[2].gl_Position);

  uvec2 pixa = uvec2(uint(trunc(W*a.x)),uint(trunc(H*a.y)));
  uvec2 pixb = uvec2(uint(trunc(W*b.x)),uint(trunc(H*b.y)));
  uvec2 pixc = uvec2(uint(trunc(W*c.x)),uint(trunc(H*c.y)));
  if (a.y<b.y) {vec3 t = b; b = a; a = t;}
  if (a.y<c.y) {vec3 t = c; c = a; a = t;}
  if (b.x<c.x) {vec3 t = b; b = c; c = t;}
  for (uint i = max(0,min(min(pixa.x,pixb.x),pixc.x));i<min(W,max(max(pixa.x,pixb.x),pixc.x)+1);i++)
  {
    for (uint j = max(0,min(min(pixa.y,pixb.y),pixc.y));j<min(H,max(max(pixa.y,pixb.y),pixc.y)+1);j++)
    {
      uint link = head_data[j*W+i];
      while (link!=0)
      {
        vec2 p = nodes[link].lp_pos;

        vec4 bar = barycentric(p,a.xy,b.xy,c.xy);
        if (bar.w>0.0001)
        {
          float d = bar.x*a.z+bar.y*b.z+bar.z*c.z;
          uvec2 scr_p = nodes[link].vp_pos;
          atomicMin(depth[scr_p.y*W+scr_p.x],uint(1000*1000*1000*d));
        }
        link = nodes[link].link;
      }
      //atomicMin(depth[j*W+i],uint(1000*1000*1000));
    }
  }
  
  gl_Position = vec4(MVP*gl_in[0].gl_Position);
  EmitVertex();
  gl_Position = vec4(MVP*gl_in[1].gl_Position);
  EmitVertex();
  gl_Position = vec4(MVP*gl_in[2].gl_Position);
  EmitVertex();
  EndPrimitive();
}