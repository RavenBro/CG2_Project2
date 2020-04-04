#pragma once

#include "RenderPass.hpp"

namespace renderer {

class IndirectLightPass : public Filter {
public:
    virtual void OnInit() override;
    virtual void Process() override;
private:
    struct {
        GLint rsmSamples, worldTex, worldNormTex;
        struct {
            GLint viewProj, worldTex, fluxTex, normalTex;
        } lights[MAX_RSM_LIGHT_COUNT];
    } uniforms;
    GLuint worldNormTex = 0, worldTex = 0;
};

};