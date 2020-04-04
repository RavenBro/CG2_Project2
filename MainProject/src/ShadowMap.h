#pragma once
#include <random>
#include <LiteMath.h>
#include "framework/ShaderProgram.h"
#include "framework/HelperGL.h"
#include <glad/glad.h>

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

        glGenTextures(1, &tempTex);
        glBindTexture(GL_TEXTURE_2D, tempTex);
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
        glDeleteTextures(1, &tempTex);
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
    GLuint GetTempTex() const { return tempTex; }
private:
    GLuint frameBuffer = 0;
    GLuint texture = 0, depthTex = 0, tempTex = 0;
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

class ReflectiveShadowMap {
public:
    void Init(int w, int h){
        width = w;
        height = h;

        glGenTextures(1, &normalTex);
        glBindTexture(GL_TEXTURE_2D, normalTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    
        GL_CHECK_ERRORS;

        glGenTextures(1, &depthTex);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        GL_CHECK_ERRORS;

        glGenTextures(1, &posTex);
        glBindTexture(GL_TEXTURE_2D, posTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERRORS;

        glGenTextures(1, &fluxTex);
        glBindTexture(GL_TEXTURE_2D, fluxTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        const GLfloat color[] {0, 0, 0, 0}; 
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);   
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERRORS;

        glGenFramebuffers(1, &framebuffer);
        GL_CHECK_ERRORS;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); 
        GL_CHECK_ERRORS; 
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
        GL_CHECK_ERRORS;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fluxTex, 0);
        GL_CHECK_ERRORS;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
        GL_CHECK_ERRORS;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, posTex, 0);
        GL_CHECK_ERRORS;
        GLuint texts[] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, texts);
        std::cout << (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) << "\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        GL_CHECK_ERRORS;
    }

    ~ReflectiveShadowMap(){
        glDeleteFramebuffers(1, &framebuffer);
        GLuint texts[] = {normalTex, posTex, depthTex, fluxTex};
        glDeleteTextures(4, texts);
    }

    GLuint GetFramebuffer() const { return framebuffer; }
    GLuint GetPosTex() const { return posTex; }
    GLuint GetNormalTex() const { return normalTex; }
    GLuint GetDepthTex() const { return depthTex; }
    GLuint GetFluxTex() const { return fluxTex; }

    void BindTarget() 
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, width, height);
    }
    void Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
private:
    GLuint framebuffer = 0;
    GLuint normalTex = 0, depthTex = 0, posTex = 0, fluxTex = 0; 
    int width, height;
};