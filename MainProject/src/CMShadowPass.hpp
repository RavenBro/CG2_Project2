#pragma once

#include "RenderPass.hpp"
namespace renderer 
{

class CMShadowPass : public RenderPass {
public:
    ~CMShadowPass() {}
    virtual void OnInit() override;
    virtual void Process() override;
private:
    void drawNode(int nodeId, const float4x4& model);
    void draw(const Mesh::Primitive& prim, const float4x4 &model);
    void setFramebuffer(CMPointLight &light, int side);

    struct {
        GLint viewProj = -1, model = -1, lightPos = -1;
        GLint zfar = -1;
    } uniforms;

};

};