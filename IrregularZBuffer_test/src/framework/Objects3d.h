#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <LiteMath.h>
using LiteMath::float4x4;

class Mesh
{
  float4x4 identity;
public:
  virtual ~Mesh() {}
  virtual const std::string &GetName() const = 0;
  virtual const float4x4 &GetTransform() const { return identity; };
  virtual void Draw() = 0;
  virtual void DrawInstanced(size_t count) {
    throw std::logic_error{"Instanced draw is not implemented. You can implement it yourself."};
  }
  virtual void DrawAdjacency() 
  {
    throw std::logic_error{"Adjacency draw is not implemented. You can implement it yourself. Albert"};
  }
};

class Model
{
public:
  unsigned int vao;
  float4x4 transform = float4x4();
  virtual ~Model() {}
  virtual const std::vector<Mesh*> &GetMeshes() const = 0;
  virtual void Draw() = 0;
};