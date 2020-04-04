#pragma once

#include "Objects3d.h"
#include <glad/glad.h>
#include <LiteMath.h>

#include <vector>
#include <string>

using LiteMath::float4x4;
using std::string;
using std::vector;

class PrimitiveMesh : public Mesh
{
protected:
  string name;
  GLuint vboVertices, vboIndices, vboNormals, vboTexCoords,vao;
  size_t ind_num;
  size_t pos_num;
  size_t tc_dim;

public:
  GLuint vao_cpy;
  float4x4 model;
  uint32_t material_id;

  PrimitiveMesh() = default;
  PrimitiveMesh(const vector<float> &positions,
                const vector<float> &normals,
                const vector<float> &texcoords,
                const vector<uint32_t> &indices,
                size_t tex_cords_dim,
                size_t mat_id,
                string n);

  const string &GetName() const override;
  void Draw() override;
  void DrawInstanced(size_t count) override;

  ~PrimitiveMesh();
};

class ColorMesh : public Mesh {
public:
  
  ColorMesh() = default;
  ColorMesh(const vector<float> &verts, const vector<float> &colors, const vector<uint32_t> &indices, const string nm); 
  virtual ~ColorMesh() override;

  const string &GetName() const override;
  void Draw() override;

private:
  string name;
  int v_count = 0, i_count = 0, c_count = 0;
  GLuint vao = 0, vbo_verts, vbo_ind, vbo_colors;
};

PrimitiveMesh* CreateSimpleTriangleMesh(string name = "SimpleTriangle");
PrimitiveMesh* CreateSimplePlaneMesh(string name = "SimplePlane");
PrimitiveMesh* CreateSimpleBoxMesh(string name = "SimpleBox");
PrimitiveMesh* CreateSkyBoxMesh(string name = "SkyBox");
PrimitiveMesh* CreateQuad(string name = "Quad");