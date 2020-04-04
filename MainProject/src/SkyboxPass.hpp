#pragma once

#include "RenderPass.hpp"

namespace renderer
{

class SkyboxPass : public RenderPass {
public:
    virtual void OnInit() override;
    virtual void Process() override;
    void SetTextures(GLuint depth, GLuint color, GLuint skyboxTex, int4 viewport);
private:
    struct {
        GLint projection, view, skybox;
    } uniforms;
    GLuint framebuffer = 0;
    GLuint skybox = 0; 
    int4 screenViewport;
    GLuint resultTex = 0;
    int count = 0;
};

};