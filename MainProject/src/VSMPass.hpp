#pragma once

#include "RenderPass.hpp"
#include "BlurPass.hpp"

namespace renderer 
{

class VSMPass : public RenderPass {
public:
    virtual ~VSMPass() {}
    virtual void OnInit() override;
    virtual void Process() override;
private:
    void drawNode(int nodeId, const float4x4& model);
    void draw(const Mesh::Primitive& prim, const float4x4 &model);
    
    void SetFB(VSMLight &light);

    struct {
        GLint MVP = -1;
    } uniforms;
    BlurPass blur;
};

};