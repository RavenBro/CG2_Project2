#pragma once

#include <LiteMath.h>
#include <glad/glad.h>
#include "framework/HelperGL.h"

using namespace LiteMath;

class ShadowMap {
public:
    ShadowMap(int width, int height) : texWidth(width), texHeight(height) 
    {
        glGenFramebuffers(1, &frameBuffer);
        
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,  
            width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        GL_CHECK_ERRORS;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    ~ShadowMap() 
    {
        glDeleteFramebuffers(1, &frameBuffer);
        glDeleteTextures(1, &depthTexture);
    }

    void BindTarget()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, texWidth, texHeight);
        GL_CHECK_ERRORS;
    }
    
    void Unbind(int width, int height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }
    GLuint GetTexture() const { return depthTexture; }
private:
    GLuint frameBuffer = 0;
    GLuint depthTexture = 0;
    int texWidth, texHeight;
};


class VarianceShadowMap{
public:
    VarianceShadowMap(int w, int h) : texWidth(w), texHeight(h)
    {
        glGenFramebuffers(1, &frameBuffer);
        
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F,  
            texWidth, texHeight, 0, GL_RG, GL_FLOAT, (void*)0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenTextures(1, &depthTex);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,  
            texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
        glReadBuffer(GL_NONE);

        GL_CHECK_ERRORS;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~VarianceShadowMap()
    {
        glDeleteTextures(1, &texture);
        glDeleteTextures(1, &depthTex);
        glDeleteFramebuffers(1, &frameBuffer);
    }

    void BindTarget()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, texWidth, texHeight);
        GL_CHECK_ERRORS;
    }
    
    void Unbind(int width, int height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }
    GLuint GetTexture() const { return texture; }
private:
    GLuint frameBuffer = 0;
    GLuint texture = 0, depthTex = 0;
    int texWidth, texHeight;
};
class CascadedShadowMap{
public:
    CascadedShadowMap(int w, int h) : texWidth(w), texHeight(h)
    {
        glGenFramebuffers(1, &frameBuffer);
        
        glGenTextures(1, &depthTex);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
             texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        for (uint i = 0 ; i < cas_num ; i++) 
        {
            glGenTextures(1, &cascadeTex[i]);
            glBindTexture(GL_TEXTURE_2D, cascadeTex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F,  
            texWidth, texHeight, 0, GL_RG, GL_FLOAT, (void*)0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
 

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);
        glBindTexture(GL_TEXTURE_2D, cascadeTex[0]);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cascadeTex[0], 0);
        glReadBuffer(GL_NONE);
    
        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    
        if (Status != GL_FRAMEBUFFER_COMPLETE) 
        {
            printf("FB error, status: 0x%x\n", Status);
        }

        GL_CHECK_ERRORS;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~CascadedShadowMap()
    {
        glDeleteTextures(1, &depthTex);
        for (uint i = 0 ; i < cas_num ; i++) 
        {
            glDeleteTextures(1, &cascadeTex[i]);
        }
        glDeleteFramebuffers(1, &frameBuffer);
    }

    void BindTarget()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, texWidth, texHeight);
        GL_CHECK_ERRORS;
    }
    
    void Unbind(int width, int height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }
    GLuint GetCascade(int num ) const { return(num<cas_num ? cascadeTex[num] : 0); }
private:
    GLuint frameBuffer = 0;
    const int cas_num = 3;
    GLuint depthTex = 0, cascadeTex[3] = {0,0,0};
    int texWidth, texHeight;
};
class FrameBuffer {
public:
    FrameBuffer(){
        glGenFramebuffers(1, &buff);
        GL_CHECK_ERRORS;
    }
    ~FrameBuffer(){
        glDeleteFramebuffers(1, &buff);
    }
    GLuint Get() const { return buff; }

    void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, buff); }
    void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
private:
    GLuint buff = 0;
};

class TextureMap
{
public:
    TextureMap(int w, int h) : texWidth(w), texHeight(h)
    {
        glGenFramebuffers(1, &frameBuffer);
        
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  
            texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, (void*)0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenTextures(1, &normalTex);
        glBindTexture(GL_TEXTURE_2D, normalTex);
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,  
            texWidth, texHeight, 0, GL_RGB, GL_FLOAT, (void*)0));
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenTextures(1, &depthTex);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,  
            texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
        glBindTexture(GL_TEXTURE_2D, normalTex);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normalTex, 0);
        glReadBuffer(GL_NONE);
        GLuint dbuff[]= {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2,dbuff);
        GL_CHECK_ERRORS;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~TextureMap()
    {
        glDeleteTextures(1, &texture);
        glDeleteTextures(1, &depthTex);
        glDeleteTextures(1, &normalTex);
        glDeleteFramebuffers(1, &frameBuffer);
    }

    void BindTarget()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, texWidth, texHeight);
        GL_CHECK_ERRORS;
    }
    
    void Unbind(int width, int height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }
    GLuint GetTexture() const { return texture; }
    GLuint GetNormTex() const { return normalTex;}
    GLuint GetDepthTex() const { return depthTex;}
private:
    GLuint frameBuffer = 0;
    GLuint texture = 0,//текстура, исходные цвета объектов
     depthTex = 0,//карта глубины
     normalTex = 0,//нормали точек в пространстве камеры
     posTex = 0;//позиции точек в пространстве камеры (до умножения на projection) пока не создана
    int texWidth, texHeight;
};
class PostEffectsMap
{
public:
    PostEffectsMap(int w, int h) : texWidth(w), texHeight(h)
    {
        glGenFramebuffers(1, &frameBuffer);
        GL_CHECK_ERRORS;
        glGenTextures(1, &ssao_tex);
        glBindTexture(GL_TEXTURE_2D, ssao_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,  
            texWidth, texHeight, 0, GL_RGB, GL_FLOAT, (void*)0);
        GL_CHECK_ERRORS;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERRORS;
        glGenTextures(1, &bloom_tex);
        glBindTexture(GL_TEXTURE_2D, bloom_tex);
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,  
            texWidth, texHeight, 0, GL_RGB, GL_FLOAT, (void*)0));
        GL_CHECK_ERRORS;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERRORS;
        glBindFramebuffer(GL_FRAMEBUFFER,frameBuffer);
        glBindTexture(GL_TEXTURE_2D, ssao_tex);GL_CHECK_ERRORS;
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ssao_tex, 0);GL_CHECK_ERRORS;
        glBindTexture(GL_TEXTURE_2D, bloom_tex);GL_CHECK_ERRORS;
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, bloom_tex, 0);GL_CHECK_ERRORS;
        glReadBuffer(GL_NONE);GL_CHECK_ERRORS;
        GL_CHECK_ERRORS;
        GLuint dbuff[]= {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2,dbuff);
        GL_CHECK_ERRORS;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~PostEffectsMap()
    {
        glDeleteTextures(1, &ssao_tex);
        glDeleteTextures(1, &bloom_tex);
        glDeleteFramebuffers(1, &frameBuffer);
    }

    void BindTarget()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, texWidth, texHeight);
        GL_CHECK_ERRORS;
    }
    
    void Unbind(int width, int height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }
    GLuint GetSSAOTex() const { return ssao_tex; }
    GLuint GetBloomTex() const { return bloom_tex;}
private:
    GLuint frameBuffer = 0;
    GLuint ssao_tex = 0,bloom_tex = 0;
    int texWidth, texHeight;
};