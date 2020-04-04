#include "Renderers.h"
void BloomRenderer::Init(int w,int h)
    {
        texWidth = w;
        texHeight = h;
        glGenFramebuffers(1, &FrameBuffer);

        glGenTextures(1, &BloomTex);
        glBindTexture(GL_TEXTURE_2D, BloomTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, BloomTex, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
        GL_CHECK_ERRORS;
    }
    BloomRenderer::~BloomRenderer()
    {
        glDeleteTextures(1, &BloomTex);
        glDeleteFramebuffers(1, &FrameBuffer);
    }
    void BloomRenderer::SetTexture(GLuint color)
    {
        colorTex = color;
    }
    void BloomRenderer::Render()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
        glViewport(0, 0, texWidth, texHeight);
        auto shader = GetShader();
        
        shader->StartUseShader();
        shader->SetUniform("tex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        DrawObjects();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }