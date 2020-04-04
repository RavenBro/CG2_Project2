#pragma once

#include "RenderPass.hpp"

namespace renderer 
{

class SMPass : public RenderPass {
public:
    ~SMPass();
    virtual void OnInit() override;
    virtual void Process() override;
private:
    void drawNode(int nodeId, const float4x4& model);
    void draw(const Mesh::Primitive& prim, const float4x4 &model);

    struct {
        GLint MVP = -1;
    } uniforms;
};

};