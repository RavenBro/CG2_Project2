#include "IndirectLightPass.hpp"
#include "RenderPass.hpp"
#include "Renderer.hpp"

namespace renderer {

void IndirectLightPass::OnInit(){
    SetShader("indirectlight_s");
    SetViewport(GetRenderer()->GetScreenViewport());
    const int samples_count = 50;
    float samples[samples_count * 2];
    srand(322);
    for(int i = 0; i < samples_count; i++){
        float ksi1 = rand() /((float)RAND_MAX);
        float ksi2 = 2*M_PI* rand() /((float)RAND_MAX);
        samples[2*i] = ksi1 * sin(ksi2);
        samples[2*i + 1] = ksi1 * cos(ksi2); 
    }
    
    BindShader();
    uniforms.rsmSamples = TryGetUniformLoc("rsmSamples");
    glUniform2fv(uniforms.rsmSamples, samples_count, samples);
    uniforms.worldNormTex = TryGetUniformLoc("worldNormTex");
    uniforms.worldTex = TryGetUniformLoc("worldTex");

    glUniform1i(uniforms.worldTex, 0);
    glUniform1i(uniforms.worldNormTex, 1); 

    for(int i = 0; i < MAX_RSM_LIGHT_COUNT; i++){
        std::string prefix = "lights[" + std::to_string(i) + "].";
        uniforms.lights[i].fluxTex = TryGetUniformLoc(prefix + "fluxTex");
        uniforms.lights[i].normalTex = TryGetUniformLoc(prefix + "normalTex");
        uniforms.lights[i].viewProj = TryGetUniformLoc(prefix + "viewProj");
        uniforms.lights[i].worldTex = TryGetUniformLoc(prefix + "worldTex");

        glUniform1i(uniforms.lights[i].fluxTex, 2 + 3*i);
        glUniform1i(uniforms.lights[i].normalTex, 3 + 3*i + 1);
        glUniform1i(uniforms.lights[i].worldTex, 4 + 3*i + 2);
    }

    for(int i = 0; i < GetScene()->lights.RSMLights.GetCount(); i++){
        auto &light = GetScene()->lights.RSMLights.Get(i);
        glUniformMatrix4fv(uniforms.lights[i].viewProj, 1, GL_TRUE, mul(light.projection, light.view).L());
    }
    UnbindShader();

    worldNormTex = TryGetTexture("GBuffer/WorldNorm");
    worldTex = TryGetTexture("GBuffer/World");

    Filter::TexInfo info;
    info.fmt = HDRColorTexFormat();
    info.fbTarget = GL_COLOR_ATTACHMENT0;
    info.width = GetViewport().z;
    info.height = GetViewport().w;
    info.level = 0;
    info.name = "Tex";
    if(!SetTargetTextures({info})){
        throw(std::runtime_error("IndirectLightPass::OnInit framebuffer/texture gen error"));
    }
}

void IndirectLightPass::Process(){
    BindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, GetFramebuffer());
    Viewport();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, worldTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, worldNormTex);

    for(int i = 0; i < GetScene()->lights.RSMLights.GetCount(); i++){
        auto &light = GetScene()->lights.RSMLights.Get(i);
        glActiveTexture(GL_TEXTURE0 + 2 + 3*i);
        glBindTexture(GL_TEXTURE_2D, light.fluxTex);
        glActiveTexture(GL_TEXTURE0 + 3 + 3*i);
        glBindTexture(GL_TEXTURE_2D, light.normalTex);
        glActiveTexture(GL_TEXTURE0 + 4 + 3*i);
        glBindTexture(GL_TEXTURE_2D, light.worldTex);
    }
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    RenderQuad();
    UnbindShader();
}

};