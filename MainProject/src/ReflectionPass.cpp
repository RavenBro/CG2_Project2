#include "ReflectionPass.hpp"
#include "Renderer.hpp"

namespace renderer 
{

const int MIP_COUNT = 5;

void ReflectionPass::OnInit(){
    SetShader("reflections_s");

    auto viewport = GetRenderer()->GetScreenViewport();

    SetViewport(viewport);
    
    frameTex = TryGetTexture("PBR/Frame");
    normalTex = TryGetTexture("GBuffer/WorldNorm");
    vertexTex = TryGetTexture("GBuffer/Vertex");
    materialTex = TryGetTexture("GBuffer/Material");
    depthTex = TryGetTexture("GBuffer/Depth");
    brdfTex = TryGetTexture("BRDFLuT/Tex");
    albedoTex = TryGetTexture("GBuffer/Albedo");
    BindShader();
    uniforms.depthTex = TryGetUniformLoc("depthTex");
    uniforms.frameTex = TryGetUniformLoc("frameTex");
    uniforms.materialTex = TryGetUniformLoc("materialTex");
    uniforms.normalTex = TryGetUniformLoc("normalTex");
    uniforms.vertexTex = TryGetUniformLoc("vertexTex");
    uniforms.projection = TryGetUniformLoc("projection");
    //uniforms.invView = TryGetUniformLoc("InvView");
    uniforms.normalMat = TryGetUniformLoc("normalMat");
    //uniforms.brdfLuT = TryGetUniformLoc("brdfLuT");
    //uniforms.albedoTex = TryGetUniformLoc("albedoTex");

    glUniform1i(uniforms.frameTex, 0);
    glUniform1i(uniforms.vertexTex, 1);
    glUniform1i(uniforms.depthTex, 2);
    glUniform1i(uniforms.materialTex, 3);
    glUniform1i(uniforms.normalTex, 4);
    //glUniform1i(uniforms.brdfLuT, 5);
    //glUniform1i(uniforms.albedoTex, 6);
    UnbindShader();

    Filter::TexInfo inf, inf2;
    inf.height = GetViewport().w;
    inf.width = GetViewport().z;
    inf.level = 0;
    inf.fbTarget = GL_COLOR_ATTACHMENT0;
    inf.fmt = ReflectTexFormat();
    inf.name = "Reflect";

    inf2.height = GetViewport().w;
    inf2.width = GetViewport().z;
    inf2.level = 0;
    inf2.fbTarget = GL_COLOR_ATTACHMENT1;
    inf2.name = "Mask";
    inf2.fmt  = ReflectMaskFormat();
    GL_CHECK_ERRORS;
    
    std::cout << GetName() << " gen textures\n";
    if(!SetTargetTextures({inf, inf2})){
        throw(std::runtime_error("ReflectionPass : framebuffer/texture generation error"));
    } 
    std::cout << "tex generated\n";

    maskTex = TryGetTexture(GetName() + "/Mask");
    resTex = TryGetTexture(GetName() + "/Reflect");
    glGenerateTextureMipmap(maskTex);
    glGenerateTextureMipmap(resTex);

    blur.Init(GetRenderer(), GetScene(), GetName() + "/Blur");
    blur.SetTextures(resTex, maskTex, GetViewport().z, GetViewport().w, inf.fmt, inf2.fmt, MIP_COUNT);
}

void ReflectionPass::Process(){
    glBindFramebuffer(GL_FRAMEBUFFER, GetFramebuffer());
    glViewport(GetViewport().x, GetViewport().y, GetViewport().z, GetViewport().w);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    BindShader();
    glUniformMatrix4fv(uniforms.projection, 1, GL_TRUE, GetScene()->sceneCamera.projection.L());
    
    auto normM = transpose4x4(inverse4x4(GetScene()->sceneCamera.view));
    glUniformMatrix4fv(uniforms.normalMat, 1, GL_TRUE, normM.L());
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frameTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, vertexTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, materialTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    /*glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, brdfTex);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, albedoTex);*/

    RenderQuad();
    UnbindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    blur.Process();
}

void ReflectionPass::ReflectBlur::OnInit(){
    SetShader("reflectblur_s");
    BindShader();
    uniforms.colotTex = TryGetUniformLoc("colorTex");
    uniforms.isHorizontal = TryGetUniformLoc("isHorizontal");
    uniforms.maskTex = TryGetUniformLoc("maskTex");
    uniforms.pass = TryGetUniformLoc("pass");
    UnbindShader();
}

void ReflectionPass::ReflectBlur::SetTextures(GLuint colorTex, GLuint maskTex, int width, int height, 
    const GLTexture2DFormat &color, const GLTexture2DFormat &mask, int levelCount)
{
    srcColor = colorTex;
    srcMask = maskTex;
    sWidth = width;
    sHeight = height;
    mipCount = levelCount;

    tempColor = GenerateTexture2D(color, sWidth, sHeight, 0);
    tempMask = GenerateTexture2D(mask, sWidth, sHeight, 0);
    if(!tempMask || !tempColor){
        throw(std::runtime_error("ReflectBlur : tex gen errr"));
    }
    glGenerateTextureMipmap(tempColor);
    glGenerateTextureMipmap(tempMask);

    glGenFramebuffers(1, &framebuffer);
    BindShader();
    glUniform1i(uniforms.colotTex, 0);
    glUniform1i(uniforms.maskTex, 1);
    UnbindShader();
}

void ReflectionPass::ReflectBlur::Process(){
    GLuint buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    int width = sWidth, height = sHeight; 
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    BindShader();

    glUniform1i(uniforms.pass, 0);
    glUniform1ui(uniforms.isHorizontal, 0);
        
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tempColor, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tempMask, 0);
    glDrawBuffers(2, buffers);
    glViewport(0, 0, width, height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcColor);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, srcMask);

    GetRenderer()->GetScreenQuad()->Draw();

    glUniform1ui(uniforms.isHorizontal, 1);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcColor, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, srcMask, 0);
    glDrawBuffers(2, buffers);
    glViewport(0, 0, width, height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tempColor);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tempMask);

    GetRenderer()->GetScreenQuad()->Draw();


    for(int pass = 0; pass < mipCount; pass++){
        glUniform1i(uniforms.pass, pass);
        glUniform1ui(uniforms.isHorizontal, 0);
        
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tempColor, pass);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tempMask, pass);
        glDrawBuffers(2, buffers);
        glViewport(0, 0, width, height);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcColor);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, srcMask);

        GetRenderer()->GetScreenQuad()->Draw();

        glUniform1ui(uniforms.isHorizontal, 1);
        width /= 2;
        height /= 2;

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcColor, pass + 1);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, srcMask, pass + 1);
        glDrawBuffers(2, buffers);
        glViewport(0, 0, width, height);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcColor);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, srcMask);

        GetRenderer()->GetScreenQuad()->Draw();
    }
    UnbindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

};