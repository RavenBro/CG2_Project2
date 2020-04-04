#pragma once
#include "RenderPass.hpp"

namespace renderer {

class CaptureScene : public RenderPass {
public:
    virtual void Process() override;
    virtual void OnInit() override;
private:
    GLuint tex;
    struct {
        GLint vsm_tex;
    } uniforms;
};


};