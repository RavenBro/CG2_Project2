#pragma once

#include "RenderPass.hpp"
#include "BlurPass.hpp"
namespace renderer 
{

class AOPass : public Filter {
public:
    virtual ~AOPass();
    
    virtual void OnInit() override;
    virtual void Process() override;
private:
    BlurPass blur;

    struct {
        GLint texture0 = -1, texture1 = -1, material_tex = -1, FocalLen = -1; 
        GLint AORes = -1, InvAORes = -1, NoiseScale = -1;
    } uniforms;
    GLuint noiseTex = 0, vertexTex = 0;
};

}