#include "AOPass.hpp"
#include "Renderer.hpp"

namespace renderer
{


AOPass::~AOPass() {
    glDeleteTextures(1, &noiseTex);
}

void AOPass::OnInit(){
    SetShader("hbao_s");

    auto vp = GetRenderer()->GetScreenViewport();
    SetViewport(vp);
    noiseTex = GenerateNoiseTexture2D(32, 32);
    if(noiseTex == 0){
        throw(std::runtime_error("AOPass::OnInit: noiseTex generation error"));
    }
    RegisterTexture(noiseTex, "Noise");
    vertexTex = TryGetTexture("GBuffer/Vertex");

    BindShader();
    uniforms.texture0 = TryGetUniformLoc("texture0");
    uniforms.texture1 = TryGetUniformLoc("texture1");
    uniforms.FocalLen = TryGetUniformLoc("FocalLen");
    uniforms.AORes = TryGetUniformLoc("AORes");
    uniforms.InvAORes = TryGetUniformLoc("InvAORes");
    uniforms.NoiseScale = TryGetUniformLoc("NoiseScale");
    
    glUniform1i(uniforms.texture0, 0);
    glUniform1i(uniforms.texture1, 1);
    glUniform1i(uniforms.material_tex, 2);
    glUniform2f(uniforms.AORes, (GLfloat)vp.z, (GLfloat)vp.w);
    glUniform2f(uniforms.InvAORes, 1.f/vp.z, 1.f/vp.w);
    glUniform2f(uniforms.NoiseScale, vp.z/32.f, vp.w/32.f);
    UnbindShader();

    TexInfo inf;
    inf.fbTarget = GL_COLOR_ATTACHMENT0;
    inf.width = vp.z;
    inf.height = vp.w;
    inf.level = 0;
    inf.fmt = HDRColorTexFormat();
    inf.name = "Tex";
    if(!SetTargetTextures({inf})){
        throw(std::runtime_error("AOPass: framebuffer/texture generation error"));
    }

    blur.Init(GetRenderer(), GetScene(), "Blur");
    blur.SetTexture(TryGetTexture(GetName() + "/Tex"), inf.fmt, inf.width, inf.height, 0);
    blur.SetSigma(3.f);
}

static float2 calcFocalLen(float fovRad, float width, float height){
    float2 res;
    res.x = 1.f/tanf(0.5 * fovRad)*(height/width);
    res.y = 1.f/tanf(0.5 * fovRad);
    return res;
}

void AOPass::Process(){
    glBindFramebuffer(GL_FRAMEBUFFER, GetFramebuffer());
    Viewport();
    BindShader();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vertexTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, noiseTex);

    auto FocalLen = calcFocalLen(GetScene()->sceneCamera.fovRad, GetViewport().z, GetViewport().w);
    glUniform2f(uniforms.FocalLen, FocalLen.x, FocalLen.y);
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT);
    RenderQuad();
    UnbindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    blur.Process();
}

};
