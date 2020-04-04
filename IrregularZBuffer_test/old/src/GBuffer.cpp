#include "GBuffer.h"
#include <cmath>
#include <glad/glad.h>
#include <iostream>
#include <random>
#include <memory>
#include <algorithm>
#include <LiteMath.h>
GBuffer::~GBuffer() 
{
    GLuint textures[] {colorTex, normalTex, depthTex, vertexTex, ssaoTex,
     noiseTex, bloomTex, materialTex,resultTex, shadedTex}; 
    glDeleteTextures(10, textures);
    glDeleteFramebuffers(1, &framebuffer);
}

bool GBuffer::Init(int width, int height)
{
    std::default_random_engine generator;
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); 
    std::vector<LiteMath::float3> ssaoNoise;
    for (unsigned int i = 0; i < 1024; i++)
    {
        LiteMath::float3 noise(
            randomFloats(generator), 
            randomFloats(generator), 
            0.0f); 
        ssaoNoise.push_back(noise);
        //std::cout<<noise.x;
    }  

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
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

    glGenTextures(1, &noiseTex);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, 32, 32, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
    GL_CHECK_ERRORS;

    glGenTextures(1, &bloomTex);
    glBindTexture(GL_TEXTURE_2D, bloomTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &shadedTex);
    glBindTexture(GL_TEXTURE_2D, shadedTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
    //UnlinkTextures();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, vertexTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, shadedTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, materialTex, 0);
    GL_CHECK_ERRORS;
    GLuint dbuff[]= {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
         GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
    glDrawBuffers(5, dbuff);
}

void GBuffer::SetLightingState()
{
    //UnlinkTextures();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloomTex, 0);
    GLuint dbuff[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, dbuff);
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
void GBuffer::UnlinkTextures()
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,  0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, 0, 0);
    GL_CHECK_ERRORS;
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
}

void PostBuffer::Init(int width, int height)
{
    winW = width;
    winH = height;

    glGenTextures(1, &lightTex);
    glBindTexture(GL_TEXTURE_2D, lightTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, winW, winH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, winW, winH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;

    glGenTextures(1, &tempTex);
    glBindTexture(GL_TEXTURE_2D, tempTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, winW, winH, 0, GL_RGBA, GL_FLOAT, nullptr);
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
