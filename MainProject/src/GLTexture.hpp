#pragma once

#include <glad/glad.h>
#include "framework/HelperGL.h"

namespace renderer 
{

struct GLTexture2DFormat {
    GLint minFilter = GL_LINEAR;
    GLint magFilter = GL_LINEAR;
    bool compareRefToTex = false;
    GLint compareFunc = GL_NEVER;
    GLint baseLevel = 0, maxLevel = 1000;
    GLfloat borderColor[4] {0.f, 0.f, 0.f, 0.f};
    GLint wrapS = GL_REPEAT, wrapT = GL_REPEAT;
    GLint internalFmt = GL_RGBA;
    GLint format = GL_RGBA;
    GLenum pixelType = GL_FLOAT;
};

GLTexture2DFormat NormalTexFormat();
GLTexture2DFormat ColorTexFormat();
GLTexture2DFormat HDRColorTexFormat();
GLTexture2DFormat VertexTexFormat();
GLTexture2DFormat DepthTexFormat();
GLTexture2DFormat AOTexFormat();
GLTexture2DFormat SMTexFormat();
GLTexture2DFormat VSMTexFormat();
GLTexture2DFormat WorldTexFormat();
GLTexture2DFormat BRDFTexFormat();
GLTexture2DFormat ReflectTexFormat();
GLTexture2DFormat ReflectMaskFormat();

GLuint GenerateTexture2D(const GLTexture2DFormat &format, int width, int height, int level, void *data = nullptr);
GLuint GenerateNoiseTexture2D(int width, int height);

};