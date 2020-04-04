#pragma once

#include <glad/glad.h>
#include "framework/HelperGL.h"

class GBuffer
{
public:
    GBuffer(){}
    ~GBuffer();
    bool Init(int width, int height);

    GLuint GetColorTex() { return colorTex; }
    GLuint GetDepthTex() { return depthTex; }
    GLuint GetNormalTex() { return normalTex; }
    GLuint GetVertexTex() { return vertexTex; }
    GLuint GetSSAOTex() { return ssaoTex; }
    GLuint GetMaterialTex() { return materialTex; }
    GLuint GetResultTex() { return resultTex; }

    void Bind();    
    void Unbind();
    void Clear();

    void SetRenderState();
    void SetLightingState();
    void SetGLightState();
private:
    GLuint framebuffer = 0;
    GLuint colorTex = 0, normalTex = 0, depthTex = 0, vertexTex = 0, ssaoTex = 0,
           materialTex = 0, resultTex = 0;
    int windowW = 0, windowH = 0;  
};

class PostBuffer
{
public:
    void Init(int width, int height);

    void Bind();
    void Unbind();

    GLuint GetLightTex() { return lightTex; }

private:
    GLuint framebuffer = 0;
    GLuint lightTex = 0;
    int winW, winH;
};

class ReflectBuff
{
public:
    void Init(int width, int height);

    void Bind();
    void Unbind();

    GLuint GetReflectionsTex() { return uvTex; }
    GLuint GetTempTex() { return tempTex; }

private:
    GLuint framebuffer = 0;
    GLuint uvTex = 0, tempTex = 0;
    int winW, winH;   
};

class HiZBuffer 
{
public:
    void Init(int width, int height);

    void Bind(int level);
    void Unbind();

    GLuint GetHiZTex() const;
    int GetMipCount() const;

private:
    GLuint framebuffer = 0;
    GLuint hizTex = 0;
    int width, height;
};