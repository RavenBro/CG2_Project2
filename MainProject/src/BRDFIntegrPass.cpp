#include "BRDFIntegrPass.hpp"
#include "Renderer.hpp"

namespace renderer {

const int TEX_SIZE = 512;

void BRDFIntegrPass::OnInit(){
    SetShader("integratebrdf_s");
    TexInfo info;
    info.name = "Tex";
    info.height = info.width = TEX_SIZE;
    info.level = 0;
    info.fmt = BRDFTexFormat();
    info.fbTarget = GL_COLOR_ATTACHMENT0;
    if(!SetTargetTextures({info})){
        throw(std::runtime_error("BRDFIntegrPass::OnInit : texture/framebuffer gen error"));
    }
    SetViewport(int4(0, 0, TEX_SIZE, TEX_SIZE));
}

void BRDFIntegrPass::Process(){
    BindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, GetFramebuffer());
    Viewport();
    RenderQuad();
    UnbindShader();
}

};