#include "BlurPass.hpp"
#include "Renderer.hpp"

namespace renderer 
{

static const int COEF_COUNT = 15; 

void BlurPass::OnInit(){
    SetShader("blur_s");
    BindShader();
    uniforms.isHorizontal = TryGetUniformLoc("isHorizontal");
    uniforms.weight = TryGetUniformLoc("weight");
    uniforms.srcTex = TryGetUniformLoc("src_tex");
    uniforms.weights = TryGetUniformLoc("weights");
    UnbindShader();
}

void BlurPass::Process(){
    BindShader();
    glUniform1ui(uniforms.isHorizontal, 0);
    glUniform1i(uniforms.srcTex, 0);
    glUniform1fv(uniforms.weight, std::min((std::size_t)COEF_COUNT, coef.size()), coef.data());
    glUniform1i(uniforms.weights, std::min((std::size_t)COEF_COUNT, coef.size()));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, targetTex);

    glBindFramebuffer(GL_FRAMEBUFFER, fbpass1);
    glViewport(0, 0, texW, texH);
    GetRenderer()->GetScreenQuad()->Draw();

    glBindFramebuffer(GL_FRAMEBUFFER, fbpass2);
    glViewport(0, 0, texW, texH);

    glUniform1ui(uniforms.isHorizontal, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tempTex);
    GetRenderer()->GetScreenQuad()->Draw();
    UnbindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlurPass::SetTexture(GLuint texture, const GLTexture2DFormat &fmt, int width, int height, int level){
    targetTex = texture;
    if(tempTex != 0){
        glDeleteTextures(1, &tempTex);
    }
    tempTex = GenerateTexture2D(fmt, width, height, level);
    if(tempTex == 0){
        throw(std::runtime_error("BlurPass : temp texture creation error"));
    }
    texW = width;
    texH = height;
    if(!fbpass1){
        glGenFramebuffers(1, &fbpass1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbpass1);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tempTex, level);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(!fbpass2){
        glGenFramebuffers(1, &fbpass2);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbpass2);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, targetTex, level);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    if(glCheckNamedFramebufferStatus(fbpass1, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ||
       glCheckNamedFramebufferStatus(fbpass2, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw(std::runtime_error("BlurPass - framebuffers incomplete"));
    }
}

void BlurPass::SetSigma(float sigma){
    int size = 3*sigma;
    coef.resize(size);
    for(int i = 0; i < size; i++) {
        coef[i] = 1.f/std::sqrt(2.f * M_PI * sigma *sigma) * std::exp(-(i*i)/(2.f * sigma * sigma));
    }
}

void BlurPass::Clear(){
    GLuint fb[] {fbpass1, fbpass2};
    glDeleteFramebuffers(2, fb);
    fbpass1 = fbpass2 = 0;
    DeleteTempTex();
}

void BlurPass::DeleteTempTex(){
    glDeleteTextures(1, &tempTex);
    tempTex = 0;
}

void BlurPass::ResetTexture(GLuint texture, int level){
    targetTex = texture;
    glBindFramebuffer(GL_FRAMEBUFFER, fbpass2);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, targetTex, level);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

};