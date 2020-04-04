#version 330 core
layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices = 12) out;

uniform mat4 projection = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 model = mat4(1);

uniform vec3 light = vec3(-15.0,2.15,0.1);
uniform vec3 campos;
out vec4 color;
uniform int front = 1;
uniform float sc=50;
#define EPS 0.001
vec3 calcNormal ( int i0, int i1, int i2 )
{
    vec3 va = gl_in [i1].gl_Position.xyz - gl_in [i0].gl_Position.xyz;
    vec3 vb = gl_in [i2].gl_Position.xyz - gl_in [i0].gl_Position.xyz;
    
    return normalize ( cross ( va, vb ) );
}
vec3 vecNorm(vec4 v1,vec4 v2, vec4 v3)
{
  vec3 va = v2.xyz - v1.xyz ;
  vec3 vb = v3.xyz  - v1.xyz ;
  return normalize ( cross ( va, vb ) );
}
vec4 proj(vec3 light_pos,vec4 pos, float scale)
{
  vec3 dir = pos.xyz - light_pos;
  return vec4(pos.xyz +dir*(scale/length(dir)),1.0);
}
void main()
{
    vec4 v0 = model*gl_in[0].gl_Position;
    vec4 v1 = model*gl_in[1].gl_Position;
    vec4 v2 = model*gl_in[2].gl_Position;
    vec4 v3 = model*gl_in[3].gl_Position;
    vec4 v4 = model*gl_in[4].gl_Position;
    vec4 v5 = model*gl_in[5].gl_Position;

    vec4 norm4=vec4(vecNorm(v0,v2,v4),1.0);
    float c = dot(norm4.xyz,light)/3;
    if (c>0)
    {
      vec3    n1 = vecNorm( v1, v2, v0 );
      vec3    n2 = vecNorm( v3, v4, v2 );
      vec3    n3 = vecNorm( v5, v0, v4 );

      bool s1,s2,s3;
      s1 = (dot(n1,light)<0);
      s2 = (dot(n2,light)<0);
      s3 = (dot(n3,light)<0);

      vec4 p0 = proj(light,v0,sc);
      vec4 p2 = proj(light,v2,sc);
      vec4 p4 = proj(light,v4,sc);

      bool f1,f2,f3,f4,f5;
      f1 = (dot(v0.xyz - campos,vecNorm(v0,v2,p0))>0);
      f2 = (dot(v2.xyz - campos,vecNorm(v2,p4,p2))>0);
      f3 = (dot(v4.xyz - campos,vecNorm(v4,v0,p4))>0);
      f4 = (dot(v0.xyz - campos,vecNorm(v0,v4,v2))>0);
      f5 = (dot(p0.xyz - campos,vecNorm(p0,p2,p4))>0);

      v0 = projection*view*v0;
      v2 = projection*view*v2;
      v4 = projection*view*v4;

      p0 = projection*view*p0;
      p2 = projection*view*p2;
      p4 = projection*view*p4;

      if (front==0)
      {
        f1=(f1==false);
        f2=(f2==false);
        f3=(f3==false);
      }
      s1=s1&&f1;
      s2=s2&&f2;
      s3=s3&&f3;

      if (s1&&s2&&s3)
      {
        gl_Position = v0;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = p0;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = v2;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
        gl_Position = p2;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
        gl_Position = v4;
        color = vec4(0.0,0.0,1.0,1.0);
        EmitVertex();
        gl_Position = p4;
        color = vec4(0.0,0.0,1.0,1.0);
        EmitVertex();
        gl_Position = v0;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = p0;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
      }
      else if (s1&&s2)
      {
        gl_Position = v0;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = p0;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = v2;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = p2;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = v4;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = p4;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
      }
      else if (s2&&s3)
      {
        gl_Position = v2;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
        gl_Position = p2;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
        gl_Position = v4;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
        gl_Position = p4;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
        gl_Position = v0;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
        gl_Position = p0;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
      }
      else if (s1&&s3)
      {
        gl_Position = v2;
        color = vec4(0.0,0.0,1.0,1.0);
        EmitVertex();
        gl_Position = p2;
        color = vec4(0.0,0.0,1.0,1.0);
        EmitVertex();
        gl_Position = v0;
        color = vec4(0.0,0.0,1.0,1.0);
        EmitVertex();
        gl_Position = p0;
        color = vec4(0.0,0.0,1.0,1.0);
        EmitVertex();
        gl_Position = v4;
        color = vec4(0.0,0.0,1.0,1.0);
        EmitVertex();
        gl_Position = p4;
        color = vec4(0.0,0.0,1.0,1.0);
        EmitVertex();
      }
      else if(s1)
      {
        gl_Position = v0;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = p0;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = v2;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = p2;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
      }
      else if(s2)
      {
        gl_Position = v2;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = p2;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = v4;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
        gl_Position = p4;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
      }
      else if(s3)
      {
        gl_Position = v4;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = p4;
        color = vec4(1.0,0.0,0.0,1.0);
        EmitVertex();
        gl_Position = v0;
        color = vec4(1.0,1.0,0.0,1.0);
        EmitVertex();
        gl_Position = p0;
        color = vec4(0.0,1.0,0.0,1.0);
        EmitVertex();
      }
    
    EndPrimitive();
    
    if ((front==1)==f4)
    {
      norm4=vec4(c,0,0,1.0);
      gl_Position = v0;
      color =  vec4(0.7,0.7,0.7,1.0);
      EmitVertex();

      gl_Position = v2;
      color =  vec4(0.7,0.7,0.7,1.0);
      EmitVertex();
      
      gl_Position = v4;
      color =  vec4(0.7,0.7,0.7,1.0);
      EmitVertex();
      EndPrimitive();
    }
    
    if ((front==1)==f5)
    {
      gl_Position = p0;
      color =  vec4(0.3,0.3,0.3,1.0);
      EmitVertex();

      gl_Position = p2;
      color =  vec4(0.3,0.3,0.3,1.0);
      EmitVertex();
      
      gl_Position = p4;
      color =  vec4(0.3,0.3,0.3,1.0);
      EmitVertex();
      EndPrimitive();
    }
    }
    else
    {

    }
}