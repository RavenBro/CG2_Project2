#include "Primitives.h"
#include "HelperGL.h"
#include "../../stb-master/stb_image.h"

PrimitiveMesh *CreateSimpleTriangleMesh(string name)
{
  vector<float> pos{
      -1, 1, -0.5,
      -1, -1, -0.5,
      1, -1, -0.5};

  vector<float> norm{
      0, 0, 1,
      0, 0, 1,
      0, 0, 1};

  vector<float> texc{
      0, 1,
      0, 0,
      1, 0};

  vector<uint32_t> ind{
      0, 1, 2};

  return new PrimitiveMesh(pos, norm, texc, ind, 2, -1, name);
}

PrimitiveMesh *CreateSimpleBoxMesh(string name)
{
  vector<float> pos{
      1,1,1,
      1,1,-1,
      1,-1,1,
      1,-1,-1,
      -1,1,1,
      -1,1,-1,
      -1,-1,1,
      -1,-1,-1};
  vector<float> norm{
      0,0,1,
      0,0,-1,
      0,0,1,
      0,0,-1,
      0,0,1,
      0,0,-1,
      0,0,1,
      0,0,-1};

  vector<float> texc{};

  vector<uint32_t> ind{
      0, 1, 2, 1, 2, 3, //xpos
      0, 1, 4, 1, 4, 5, //ypos
      0, 2, 4, 2, 4, 6, //zpos
      7, 6, 5, 6, 5, 4, //xneg
      7, 6, 3, 6, 3, 2, //yneg
      7, 5, 3, 5, 3, 1};//zneg

  return new PrimitiveMesh(pos, norm, texc, ind, -1,  -1, name);
}





PrimitiveMesh *CreateSkyBoxMesh(string name)
{
  vector<float> pos{
      -20.0f,  20.0f, -20.0f,
    -20.0f, -20.0f, -20.0f,
     20.0f, -20.0f, -20.0f,
     20.0f, -20.0f, -20.0f,
     20.0f,  20.0f, -20.0f,
    -20.0f,  20.0f, -20.0f,

    -20.0f, -20.0f,  20.0f,
    -20.0f, -20.0f, -20.0f,
    -20.0f,  20.0f, -20.0f,
    -20.0f,  20.0f, -20.0f,
    -20.0f,  20.0f,  20.0f,
    -20.0f, -20.0f,  20.0f,

     20.0f, -20.0f, -20.0f,
     20.0f, -20.0f,  20.0f,
     20.0f,  20.0f,  20.0f,
     20.0f,  20.0f,  20.0f,
     20.0f,  20.0f, -20.0f,
     20.0f, -20.0f, -20.0f,

    -20.0f, -20.0f,  20.0f,
    -20.0f,  20.0f,  20.0f,
     20.0f,  20.0f,  20.0f,
     20.0f,  20.0f,  20.0f,
     20.0f, -20.0f,  20.0f,
    -20.0f, -20.0f,  20.0f,

    -20.0f,  20.0f, -20.0f,
     20.0f,  20.0f, -20.0f,
     20.0f,  20.0f,  20.0f,
     20.0f,  20.0f,  20.0f,
    -20.0f,  20.0f,  20.0f,
    -20.0f,  20.0f, -20.0f,

    -20.0f, -20.0f, -20.0f,
    -20.0f, -20.0f,  20.0f,
     20.0f, -20.0f, -20.0f,
     20.0f, -20.0f, -20.0f,
    -20.0f, -20.0f,  20.0f,
     20.0f, -20.0f,  20.0f};
      


  vector<float> norm{};

  vector<float> texc{};

  vector<uint32_t> ind{};

  return  new PrimitiveMesh(pos, norm, texc, ind, -1,  -1, name);
  
}

PrimitiveMesh* CreateQuad(string name)
{
  vector<float> pos{
      -1, 1, -0.5,
      -1, -1, -0.5,
      1, -1, -0.5,
      1, 1, -0.5};

  vector<float> norm{
      0, 0, 1,
      0, 0, 1,
      0, 0, 1,
      0, 0, 1};

  vector<float> texc{
      0, 1,
      0, 0,
      1, 0,
      1, 1};
  vector<uint32_t> ind{0, 1, 2, 0, 2, 3};
  return new PrimitiveMesh(pos, norm, texc, ind, 2, -1, name);
}

PrimitiveMesh *CreateSimplePlaneMesh(string name)
{
  vector<float> pos{
      1,0,1,
      1,0,-1,
      -1,0,1,
      -1,0,-1};
  vector<float> norm{
      0,1,0,
      0,1,0,
      0,1,0,
      0,1,0,};

  vector<float> texc{};

  vector<uint32_t> ind{
      0, 1, 2, 1, 2, 3};

  return new PrimitiveMesh(pos, norm, texc, ind, -1,  -1, name);
}

PrimitiveMesh *CreateAdjacencyTestMesh(string name)
{
  vector<float> pos{
      1,1,1,
      1,1,-1,
      1,-1,1,
      1,-1,-1,
      -1,1,1,
      -1,1,-1,
      -1,-1,1,
      -1,-1,-1};
  vector<float> norm{
      1,1,1,
      1,1,-1,
      1,-1,1,
      1,-1,-1,
      -1,1,1,
      -1,1,-1,
      -1,-1,1,
      -1,-1,-1};

  vector<float> texc{};

  vector<uint32_t> ind{
      1,4,0,4,2,3,1,0,2,6,3,5,0,2,1,5,4,2,4,0,1,3,5,6,2,1,0,1,4,6,2,0,4,5,6,3,7,3,6,4,5,3,5,7,6,2,4,1,6,5,7,5,3,2,6,7,3,1,2,4,7,6,5,1,3,6,3,7,5,4,1,2};

  return new PrimitiveMesh(pos, norm, texc, ind, -1,  -1, name);
}
PrimitiveMesh *CreateAdjacencyTestMesh2(string name)
{
  vector<float> pos{
      0,0,0, //0
      0,0,2, //1
      0,2,0, //2
      2,0,0, //3
      1,1,0, //4
      1,0,1, //5
      0,1,1};//6
  vector<float> norm{
      };

  vector<float> texc{};

  vector<uint32_t> ind{
      3,0,4,6,5,0, 5,3,4,2,6,1, 6,5,4,0,2,0, 1,0,5,4,6,0,
      3,5,0,2,4,5, 2,6,4,3,0,6,
      3,4,5,1,0,4, 0,3,5,6,1,6,//2
      0,1,6,4,2,4, 0,5,1,5,6,2};

  return new PrimitiveMesh(pos, norm, texc, ind, -1,  -1, name);
}
PrimitiveMesh *CreateAdjacencyTestMesh2Norm(string name)
{
  vector<float> pos{
      0,0,0, //0
      0,0,2, //1
      0,2,0, //2
      2,0,0, //3
      1,1,0, //4
      1,0,1, //5
      0,1,1};//6
  vector<float> norm{
      };

  vector<float> texc{};

  vector<uint32_t> ind{
      3,4,5, 5,4,6, 6,4,2, 1,5,6,
      3,0,4, 2,4,0,
      3,5,0, 0,5,1,
      0,6,2, 0,1,6};

  return new PrimitiveMesh(pos, norm, texc, ind, -1,  -1, name);
}
PrimitiveMesh::PrimitiveMesh(const vector<float> &positions,
                             const vector<float> &normals,
                             const vector<float> &texcoords,
                             const vector<uint32_t> &indices,
                             size_t tex_cords_dim,
                             size_t mat_id,
                             string n)
{
  
  name = n;
  tc_dim = tex_cords_dim;

  ind_num = indices.size();
  pos_num = positions.size();

  GL_CHECK( glGenVertexArrays(1, &vao) );
  GL_CHECK( glGenBuffers(1, &vboVertices) );
  GL_CHECK( glGenBuffers(1, &vboIndices) );
  GL_CHECK( glGenBuffers(1, &vboNormals) );
  GL_CHECK( glGenBuffers(1, &vboTexCoords) );

  vao_cpy = vao; 

  GL_CHECK( glBindVertexArray(vao) );
  {
    // Pass Vertex's Positions to shader
    GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, vboVertices) );
    GL_CHECK( glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GL_FLOAT), positions.data(), GL_STATIC_DRAW) );
    GL_CHECK( glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid *)0) );
    GL_CHECK( glEnableVertexAttribArray(0) );

    // Pass Vertex's Normals to shader
    if (normals.size())
    {
      GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, vboNormals) );
      GL_CHECK( glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GL_FLOAT), normals.data(), GL_STATIC_DRAW) );
      GL_CHECK( glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid *)0) );
      GL_CHECK( glEnableVertexAttribArray(1) );
    }

    // Pass Vertex's Texture Coordinates to shader
    if (texcoords.size())
    {
      GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords) );
      GL_CHECK( glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GL_FLOAT), texcoords.data(), GL_STATIC_DRAW) );
      GL_CHECK( glVertexAttribPointer(2, tc_dim, GL_FLOAT, GL_FALSE, tc_dim * sizeof(GL_FLOAT), (GLvoid *)0) );
      GL_CHECK( glEnableVertexAttribArray(2) );
    }

    // Pass Indexes to shader
    if (indices.size())
    {
      GL_CHECK( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices) );
      GL_CHECK( glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW) );
    }
  }
  GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, 0) );
  GL_CHECK( glBindVertexArray(0) );

  material_id = mat_id;
}

const string &PrimitiveMesh::GetName() const
{
  return name;
}



void PrimitiveMesh::Draw()
{
  GL_CHECK( glBindVertexArray(vao) );
  if (ind_num) glDrawElements(GL_TRIANGLES, ind_num, GL_UNSIGNED_INT, nullptr);
  else glDrawArrays(GL_TRIANGLES, 0, pos_num);
  GL_CHECK_ERRORS;
  GL_CHECK( glBindVertexArray(0) );
}

void PrimitiveMesh::DrawInstanced(size_t count)
{
  GL_CHECK( glBindVertexArray(vao) );
  if (ind_num) glDrawElementsInstanced(GL_TRIANGLES, ind_num, GL_UNSIGNED_INT, nullptr, count);
  else glDrawArraysInstanced(GL_TRIANGLES, 0, pos_num, count);
  GL_CHECK_ERRORS;
  GL_CHECK( glBindVertexArray(0) );
}
void PrimitiveMesh::DrawAdjacency()
{
  GL_CHECK( glBindVertexArray(vao) );
  if (ind_num) glDrawElements(GL_TRIANGLES_ADJACENCY, ind_num, GL_UNSIGNED_INT, nullptr);
  else glDrawArrays(GL_TRIANGLES_ADJACENCY, 0, pos_num);
  GL_CHECK_ERRORS;
  GL_CHECK( glBindVertexArray(0) );
}
PrimitiveMesh::~PrimitiveMesh()
{
  if (vao)
    GL_CHECK( glDeleteVertexArrays(1, &vao) );
}

ColorMesh::ColorMesh(const vector<float> &verts, const vector<float> &colors, const vector<uint32_t> &indices, const string nm):
  name(nm)
{
  i_count = indices.size();
  if(!i_count)
    throw std::logic_error("No indices specified\n");
  glGenBuffers(1, &vbo_colors);
  glGenBuffers(1, &vbo_ind);
  glGenBuffers(1, &vbo_verts);
  glGenVertexArrays(1, &vao);
  
  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_verts);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
  glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (GLvoid*)0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ind);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  GL_CHECK_ERRORS;
}

ColorMesh::~ColorMesh()
{
  if(vao) glDeleteVertexArrays(1, &vao);
  GL_CHECK_ERRORS;
}

void ColorMesh::Draw()
{
  glBindVertexArray(vao);
  if(i_count) glDrawElements(GL_TRIANGLES, i_count, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

const string& ColorMesh::GetName() const
{
  return name;
}