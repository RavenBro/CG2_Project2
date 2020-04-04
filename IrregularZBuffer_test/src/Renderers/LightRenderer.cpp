#include "Renderers.h"

void LightRenderer::Attach()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glViewport(0, 0, texWidth, texHeight);
}

void LightRenderer::Unattach(int w, int h)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightRenderer::Init(int w, int h)
{
    texWidth = w;
    texHeight = h;

    glGenTextures(1, &LightTexture);
    glBindTexture(GL_TEXTURE_2D, LightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;
    
    glGenFramebuffers(1, &FrameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, LightTexture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}
void LightRenderer::SetTextures(GLuint diffuseT, GLuint normalT, GLuint vertexT, GLuint materialT)
    {
        diffuseTex = diffuseT;
        normalTex = normalT;
        vertexTex = vertexT;
        materialTex = materialT;
    }

    void LightRenderer::SetLightSrc(unsigned n, float3 pos, float3 radiocity)
    {
        auto shader = GetShader();
        shader->StartUseShader();
        shader->SetUniform("LightPos[" + std::to_string(n) + "]", pos);
        shader->SetUniform("LightColor[" + std::to_string(n) + "]", radiocity);
        shader->StopUseShader();
    }

    void LightRenderer::SetView(const float4x4 &viewM)
    {
        view = viewM;
    }

    void LightRenderer::Render()
    {
        auto shader = GetShader();
        shader->StartUseShader();
        shader->SetUniform("diffuseTex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTex);
        shader->SetUniform("normalTex", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTex);
        shader->SetUniform("vertexTex", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, vertexTex);
        shader->SetUniform("materialTex", 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, materialTex);
        shader->SetUniform("view", view);
        
        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
        shader->StopUseShader();
    }
    LightRenderer::~LightRenderer()
    {
    GLuint textures[] {LightTexture}; 
    glDeleteTextures(1, textures);
    glDeleteFramebuffers(1, &FrameBuf);
    }
