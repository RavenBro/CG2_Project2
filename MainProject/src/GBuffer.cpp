#include "GBuffer.h"

GBuffer::~GBuffer() 
{
    GLuint textures[] {colorTex, normalTex, depthTex, vertexTex, ssaoTex, materialTex}; 
    glDeleteTextures(6, textures);
    glDeleteFramebuffers(1, &framebuffer);
}

bool GBuffer::Init(int width, int height)
{
    windowW = width;
    windowH = height;

    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowW, windowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &normalTex);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    

    glGenTextures(1, &vertexTex);
    glBindTexture(GL_TEXTURE_2D, vertexTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &ssaoTex);
    glBindTexture(GL_TEXTURE_2D, ssaoTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;

    glGenTextures(1, &materialTex);
    glBindTexture(GL_TEXTURE_2D, materialTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;

    glGenTextures(1, &resultTex);
    glBindTexture(GL_TEXTURE_2D, resultTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;
    
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    SetRenderState();
    return true;
}

void GBuffer::SetRenderState()
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, vertexTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, materialTex, 0);
    GLuint dbuff[]= {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, dbuff);
    //std::cout << (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) << "\n";
}

void GBuffer::SetLightingState()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTex, 0);
    GLuint dbuff[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, dbuff);
}

void GBuffer::SetGLightState()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resultTex, 0);
    GLuint dbuff[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, dbuff);
}

void GBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, windowW, windowH);
}

void GBuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::Clear()
{
    Bind();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void PostBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    //std::cout << (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) << "\n";
    glViewport(0, 0, winW, winH);
}

void PostBuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, lightTex);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void PostBuffer::Init(int width, int height)
{
    winW = width;
    winH = height;

    glGenTextures(1, &lightTex);
    glBindTexture(GL_TEXTURE_2D, lightTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, winW, winH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE2D, GL_LINEAR_MIPMAP_LINEAR, GL_
    GL_CHECK_ERRORS;
    
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightTex, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void ReflectBuff::Init(int width, int height)
{
    winW = width;
    winH = height;

    glGenTextures(1, &uvTex);
    glBindTexture(GL_TEXTURE_2D, uvTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, winW, winH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;

    glGenTextures(1, &tempTex);
    glBindTexture(GL_TEXTURE_2D, tempTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, winW, winH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;
    
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uvTex, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void ReflectBuff::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    //std::cout << (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) << "\n";
    glViewport(0, 0, winW, winH);
}
void ReflectBuff::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);    
}
