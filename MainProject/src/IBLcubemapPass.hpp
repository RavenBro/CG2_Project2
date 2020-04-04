#pragma once

#include "RenderPass.hpp"

namespace renderer
{

class IBLcubemapPass : public RenderPass {
public:
    virtual void OnInit() override;
    virtual void Process() override;
private:
    struct {
        GLint view, projection, envMap, upDir;
    } uniforms;
};

};