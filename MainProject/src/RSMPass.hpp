#pragma once
#include "RenderPass.hpp"

namespace renderer {

class RSMPass : public RenderPass {
public:
    virtual void OnInit() override;
    virtual void Process() override;
private:
    void drawNode(int nodeId, const float4x4 &model);
    void setMaterial(const Material &material);
    void draw(const Mesh::Primitive &prim, const float4x4 &model);
    struct {
        GLint model, view, projection, albedoTex, useTex, modelColor, lightColor, lightPos;
    } uniforms;
};

};