#include "AssemblePass.hpp"
#include "Renderer.hpp"

namespace renderer
{

void AssemblePass::OnInit() {
    auto renderer = GetRenderer();
    const auto& config = renderer->GetConf();
    
    SetViewport(renderer->GetScreenViewport());
    SetShader("asm_s");
    frameTex = TryGetTexture("PBR/Frame");
    materialTex = TryGetTexture("GBuffer/Material");
    IBLReflectTex = TryGetTexture("PBR/Reflect");
    FTex = TryGetTexture("PBR/F");

    if(config.useReflections){
        reflectTex = TryGetTexture("Ref/Reflect");
        reflectMask = TryGetTexture("Ref/Mask");
    }

    if(config.useBloom){
        bloomTex = TryGetTexture("PBR/Bloom");
    }
    if(config.useIndirectLighting){
        ILTex = TryGetTexture("IL/Tex");
    }
    partTex = TryGetTexture("GBuffer/Particles");
    auto prog = BindShader();
    uniforms.frameTex = TryGetUniformLoc("frameTex");
    uniforms.materialTex = TryGetUniformLoc("materialTex");
    uniforms.reflectTex = TryGetUniformLoc("reflectTex");
    uniforms.reflectMask = TryGetUniformLoc("reflectMask");
    uniforms.useReflections = TryGetUniformLoc("useReflections");
    uniforms.useBloom = TryGetUniformLoc("useBloom");
    uniforms.bloomTex = TryGetUniformLoc("bloomTex");
    uniforms.useIndirectLight = TryGetUniformLoc("useIndirectLight");
    uniforms.indirectLight = TryGetUniformLoc("indirectLight");
    uniforms.IBLReflectTex = TryGetUniformLoc("IBLReflectTex");
    uniforms.FTex = TryGetUniformLoc("FTex");
    uniforms.partTex = TryGetUniformLoc("partTex");

    glUniform1i(uniforms.frameTex, 0);
    glUniform1i(uniforms.materialTex, 1);
    glUniform1i(uniforms.reflectTex, 2);
    glUniform1i(uniforms.bloomTex, 3);
    glUniform1i(uniforms.indirectLight, 4);
    glUniform1i(uniforms.reflectMask, 5);
    glUniform1i(uniforms.IBLReflectTex, 6);
    glUniform1i(uniforms.FTex, 7);
    glUniform1i(uniforms.partTex,8);

    glUniform1ui(uniforms.useReflections, config.useReflections);
    glUniform1ui(uniforms.useIndirectLight, config.useIndirectLighting);
    glUniform1ui(uniforms.useBloom, config.useBloom);
    UnbindShader();

    Filter::TexInfo inf;
    inf.fbTarget = GL_COLOR_ATTACHMENT0;
    inf.fmt = ColorTexFormat();
    inf.level = 0;
    inf.name = "Result";
    inf.width = GetViewport().z;
    inf.height = GetViewport().w;

    if(!SetTargetTextures({inf})){
        throw(std::runtime_error("AssemblePass.cpp : texture/framebuffer creation failed"));
    }
    
}   

void AssemblePass::Process() {
    glBindFramebuffer(GL_FRAMEBUFFER, GetFramebuffer());
    glViewport(GetViewport().x, GetViewport().y, GetViewport().z, GetViewport().w);
    BindShader();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frameTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, materialTex);
    if(reflectTex){
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, reflectTex);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, reflectMask);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, IBLReflectTex);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, FTex);
    }
    if(bloomTex){
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, bloomTex);
    }
    if(ILTex){
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ILTex);
    }
    glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, partTex);
    RenderQuad();
    UnbindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

};