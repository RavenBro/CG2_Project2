#include "Renderers.h"
void ScreenReflectionsRenderer::SetTextures(GLuint frame, GLuint normal, GLuint depth,
     GLuint vertex, GLuint material, GLuint cubemap, GLuint skybox)
    {
        frameTex = frame; 
        normalTex = normal;
        vertexTex = vertex;
        materialTex = material;
        depthTex = depth;
        cubeMapTex = cubemap;
        skyBoxTex = skybox;
    }

    void ScreenReflectionsRenderer::Render(float3 camPos, float4x4 camMatr)
    {
        auto shader = GetShader();
        shader->StartUseShader();
        shader->SetUniform("campos",camPos);
        shader->SetUniform("cam",camMatr);
        shader->SetUniform("caminv",inverse4x4(camMatr));
        shader->SetUniform("projection", GetProj());
        shader->SetUniform("frameTex", 0);
        shader->SetUniform("vertexTex", 1);
        shader->SetUniform("depthTex", 2);
        shader->SetUniform("normalTex", 3);
        shader->SetUniform("materialTex", 4);
        shader->SetUniform("cubeMapTex", 5);
        shader->SetUniform("skyBoxTex", 6);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frameTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, vertexTex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, normalTex);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, materialTex);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTex);
        DrawObjects();
        shader->StopUseShader();
    }
void ScreenReflectionsRenderer::Init(int width, int height)
{
    texWidth = width;
    texHeight = height;

    glGenTextures(1, &ReflectionsTexture);
    glBindTexture(GL_TEXTURE_2D, ReflectionsTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;

    glGenTextures(1, &RefractionsTexture);
    glBindTexture(GL_TEXTURE_2D, RefractionsTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;
    
    glGenFramebuffers(1, &FrameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ReflectionsTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, RefractionsTexture, 0);
    GLuint dbuff[]= {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, dbuff);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
ScreenReflectionsRenderer::~ScreenReflectionsRenderer()
{
    GLuint textures[] {ReflectionsTexture, RefractionsTexture}; 
    glDeleteTextures(2, textures);
    glDeleteFramebuffers(1, &FrameBuf);
}
void ScreenReflectionsRenderer::Attach()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glViewport(0, 0, texWidth, texHeight);
}

void ScreenReflectionsRenderer::Unattach(int w, int h)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, w, h);
}
