#pragma once

#include <glad/glad.h>
#include "framework/HelperGL.h"

namespace renderer 
{

struct GLTexture2DFormat {
    GLint minFilter = GL_NEAREST_MIPMAP_LINEAR;
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

GLuint GenerateTexture2D(const GLTexture2DFormat &format, int width, int height, int level);

};