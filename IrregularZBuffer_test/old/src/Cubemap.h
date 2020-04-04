#pragma once

#include <vector>
#include <glad/glad.h>
#include "framework/Primitives.h"

class Skybox : public Mesh
{
public:
    Skybox(){}

    bool Load(const std::vector<std::string>& faces);
    virtual void Draw() override;

    virtual const std::string &GetName() const override { return "Cubemap"; };
    virtual ~Skybox() override { glDeleteTextures(1, &texId); }
    GLuint GetTexId() { return texId; }
private:
    GLuint texId = 0;
    std::unique_ptr<PrimitiveMesh> baseCubemap;
};