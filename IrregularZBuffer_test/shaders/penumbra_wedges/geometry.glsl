#version 330 core
layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices = 90) out;

uniform mat4 projection = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 model = mat4(1);

uniform vec3 light = vec3(-15.0,2.15,0.1);
uniform vec3 campos;
out vec4 color;
vec4 cl;
uniform int front = 1;
uniform int side = 1;
uniform float sc=50;
uniform float light_radius = 0.5;
#define EPS 0.001
vec3 wps[4];
vec3 nf[2];
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
vec3 vec3Norm(vec3 v1, vec3 v2, vec3 v3)
{
  vec3 va = v2 - v1;
  vec3 vb = v3 - v1;
  return normalize ( cross ( va, vb ) );
}
void wedge_proj(vec3 u1,vec3 u2,vec3 light_pos,float radius,float scale)
{
  /*vec3 v1 = u1 - light_pos;
  vec3 v2 = u2 - light_pos;
  float l1 = scale/length(v1);
  float l2 = scale/length(v2);
  vec3 v3 = normalize(cross(v1,v2))*radius;
  vec3 p = u1  +v1*l1;
  wps[0]=p + v3*l1;
  wps[1]=p - v3*l1;
  p = u2 + v2*l2;

  wps[2]=p + v3*l2;
  wps[3]=p - v3*l2;*/

}
void wedge_norm(vec3 u1,vec3 u2,vec3 L,float radius,float scale)
{
  vec3 v1 = u1 - L;
  vec3 v2 = u2 - L;
  float dec = (length(v1)/length(v2));
  if (dec>1) v1/=dec;
  else v2*=dec;
  vec3 vm = 0.5*v1+0.5*v2;
  vec3 ve = u2 - u1;
  vec3 rv = normalize(cross(v1,v2))*radius;
  vec3 vp = normalize(vm)*scale;
  float sin_a = radius/length(vm);
  float l = length(vm) - radius*sin_a;
  vec3 vs1 = -l/length(vm)*vm + radius*sqrt(1-sin_a*sin_a);
  vec3 vs2 = -l/length(vm)*vm - radius*sqrt(1-sin_a*sin_a);
  nf[0] = normalize(cross(ve,vs1));
  nf[1] = normalize(cross(vs2,ve));
}
void calc_wps(vec3 u0,vec3 u1, vec3 u2, vec3 u3,vec3 light_pos,float radius,float scale)
{
  wedge_norm(u1,u2,light_pos,radius,scale);
  vec3 n2 = nf[0];
  vec3 f2 = nf[1];
  //cl=vec4(n2.rrr*0.5+vec3(0.5),1.0);
  vec3 n1 = n2;
  vec3 f1 = f2;
  if (length(u0-u1)>EPS)
  {
    wedge_norm(u0,u1,light_pos,radius,scale);
    n1 = nf[0];
    f1 = nf[1];
  }
  vec3 n3 = n2;
  vec3 f3 = f2;
  if (length(u2-u3)>EPS)
  {
    wedge_norm(u2,u3,light_pos,radius,scale);
    n3 = nf[0];
    f3 = nf[1];
  }
  vec3 cr = cross(n1,n2);
  vec3 p1n,p1f,p2n,p2f;
  if (length(cr)<1000*EPS)//точки u0 u1 u2 лежат на одной прямой
  {
    p1n = scale*normalize(cross(u2-u0,n1));
  }
  else
  {
    p1n = cr;
    if (dot(cr,u1-light_pos)<0) p1n = -p1n;
  }
  cr = cross(f1,f2);
  if (length(cr)<1000*EPS)//точки u0 u1 u2 лежат на одной прямой
  {
    p1f = scale*normalize(cross(f1,u2-u0));
  }
  else
  {
    p1f = cr;
    if (dot(cr,u1-light_pos)<0) p1f = -p1f;
  }
  cr = cross(n2,n3);
  if (length(cr)<1000*EPS)//точки u1 u2 u3 лежат на одной прямой
  {
    p2n = scale*normalize(cross(u3-u1,n2));
  }
  else
  {
    p2n = cr;
    if (dot(cr,u2-light_pos)<0) p2n = -p2n;
  }
  cr = cross(f2,f3);
  if (length(cr)<EPS)//точки u1 u2 u3 лежат на одной прямой
  {
    p2f = scale*normalize(cross(f2,u3-u1));
  }
  else
  {
    p2f = cr;
    if (dot(cr,u2-light_pos)<0) p2f = -p2f;
  }
  wps[0]=u1 + p1n;
  wps[1]=u1 + p1f;
  wps[2]=u2 + p2n;
  wps[3]=u2 + p2f;
}
void wedge_create(vec3 u1,vec3 u2, vec3 light_pos,float radius,float scale)
{
  //vec3 wps[4];

  vec3 vv1 = u1 - light_pos;
  vec3 vv2 = u2 - light_pos;
  float dec = (length(vv1)/length(vv2));
  if (dec>1) vv1/=dec;
  else vv2*=dec;
  float l1 = scale/length(vv1);
  float l2 = scale/length(vv2);
  vec3 vv3 = normalize(cross(vv1,vv2))*radius;
  
  vec3 p = u1  +vv1*l1;
  vec3 vv4 = normalize(u1-u2)*radius;
  wps[0]=p + vv3*l1 + vv4*l1;
  wps[1]=p - vv3*l1 + vv4*l1;
  p = u2 + vv2*l2;

  wps[2]=p + vv3*l2 - vv4*l2;
  wps[3]=p - vv3*l2 - vv4*l2;

  //calc_wps(u0,u1,u2,u3,light_pos,radius,scale);

  mat4 PV = projection*view;
  vec4 v1 = PV *vec4(u1,1.0);
  vec4 v2 = PV *vec4(u2,1.0);
  vec4 p0 = PV *vec4(wps[0],1.0);
  vec4 p1 = PV *vec4(wps[1],1.0);
  vec4 p2 = PV *vec4(wps[2],1.0);
  vec4 p3 = PV *vec4(wps[3],1.0);

  bool f1 = (dot(wps[0] - campos,vec3Norm(u1,u2,wps[0]))>0);
  bool f2 = (dot(u2 - campos,vec3Norm(u2,wps[3],wps[2]))>0);
  bool f3 = (dot(wps[1] - campos,vec3Norm(wps[1],wps[0],wps[3]))>0);
  bool f4 = (dot(u1 - campos,vec3Norm(u1,wps[0],wps[1]))>0);
  bool f5 = (dot(u1 - campos,vec3Norm(u1,wps[1],wps[3]))>0);
  //f1 = true;
  //f2 = true;
  //f3 = true;
  //f4 = true;
  //f5 = true;
  if (((front==1)==f1))
  {
    //cl = vec4(1.0,0.0,0.0,1.0);
    gl_Position = v1;
    color = cl;
    EmitVertex();
    gl_Position = p0;
    color = cl;
    EmitVertex();
    gl_Position = v2;
    color = cl;
    EmitVertex();
    gl_Position = p2;
    color = cl;
    EmitVertex();
    EndPrimitive();
  }
  else
  {
    gl_Position = vec4(0,0,0,1);
    color = cl;
    EmitVertex();
    gl_Position = vec4(0,0,0,1);
    color = cl;
    EmitVertex();
    gl_Position = vec4(0,0,0,1);
    color = cl;
    EmitVertex();
    EndPrimitive();
  }
  if ((front==1)==f2)
  {
    //cl = vec4(0.0,1.0,0.0,1.0);
    gl_Position = v2;
    color = cl;
    EmitVertex();
    gl_Position = p2;
    color = cl;
    EmitVertex();
    gl_Position = p3;
    color = cl;
    EmitVertex();
    EndPrimitive();
  }
  else
  {
    EndPrimitive();
  }
  if (((front==1)==f3))
  {
   // cl = vec4(0.0,0.0,1.0,1.0);
    gl_Position = p2;
    color = cl;
    EmitVertex();
    gl_Position = p3;
    color = cl;
    EmitVertex();
    gl_Position = p0;
    color = cl;
    EmitVertex();
    gl_Position = p1;
    color = cl;
    EmitVertex();
    EndPrimitive();

    
  }
  else
  {
    EndPrimitive();
  }
  if (((front==1)==f4))
  {
   // cl = vec4(1.0,1.0,0.0,1.0);
    gl_Position = p0;
    color = cl;
    EmitVertex();
    gl_Position = p1;
    color = cl;
    EmitVertex();
    gl_Position = v1;
    color = cl;
    EmitVertex();
    EndPrimitive();
  }
  else
  {
    EndPrimitive();
  }
  if (((front==1)==f5))
  {
   // cl = vec4(0.0,1.0,1.0,1.0);
    gl_Position = v1;
    color = cl;
    EmitVertex();
    gl_Position = v2;
    color = cl;
    EmitVertex();
    gl_Position = p1;
    color = cl;
    EmitVertex();
    gl_Position = p3;
    color = cl;
    EmitVertex();
    EndPrimitive();
  }
  else
  {
    EndPrimitive();
  }
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
    float c = dot(norm4.xyz,light);
    if (c>EPS)
    {
      vec3    n1 = vecNorm( v1, v2, v0 );
      vec3    n2 = vecNorm( v3, v4, v2 );
      vec3    n3 = vecNorm( v5, v0, v4 );

      bool s1,s2,s3;
      s1 = (dot(n1,light)<0);
      s2 = (dot(n2,light)<0);
      s3 = (dot(n3,light)<0);

      
      vec3 v40sh = 0*normalize(v4.xyz - v0.xyz)/5000.0 + v0.xyz;
      vec3 v04sh = 0*normalize(v0.xyz - v4.xyz)/5000.0 + v4.xyz;

      vec3 v20sh = 0*normalize(v2.xyz - v0.xyz)/5000.0 + v0.xyz;
      vec3 v02sh = 0*normalize(v0.xyz - v2.xyz)/5000.0 + v2.xyz;

      vec3 v42sh = 0*normalize(v4.xyz - v2.xyz)/5000.0 + v2.xyz;
      vec3 v24sh = 0*normalize(v2.xyz - v4.xyz)/5000.0 + v4.xyz;
      vec3 u0,u3;
      if (s1&&(side==1))
      {
        cl = vec4(1.0,0.0,0.0,1.0);
        wedge_create(v02sh,v20sh,light,light_radius,sc);
        
      }
      if (s2&&(side==2))
      {
        cl= vec4(0.0,1.0,0.0,1.0);
        wedge_create(v24sh,v42sh,light,light_radius,sc);
      }
      if (s3&&(side==3))
      {
        cl= vec4(0.0,0.0,1.0,1.0);
        wedge_create(v04sh,v40sh,light,light_radius,sc);
      }
    }
    
}