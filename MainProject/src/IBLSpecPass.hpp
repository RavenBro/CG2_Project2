#pragma once
#include "RenderPass.hpp"

namespace renderer {

class IBLSpecPass : public RenderPass {
public:
    virtual void OnInit() override;
    virtual void Process() override;

private:
    struct {
        GLint envMap, roughness, projection, view;
    } uniforms;
};

};