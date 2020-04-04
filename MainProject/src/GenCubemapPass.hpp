#pragma once

#include "RenderPass.hpp"

namespace renderer{

class GenCubemapPass : public RenderPass {
public:
    virtual void OnInit() override;
    virtual void Process() override;
private:
    struct {
        GLint equirectangularMap, view, projection;
    } uniforms;
};

};