#include "GLTexture.hpp"

namespace renderer 
{

GLuint GenerateTexture2D(const GLTexture2DFormat &format, int width, int height, int level){
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, level, format.internalFmt, width, height, 0, format.format, format.pixelType, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, format.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, format.minFilter);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, format.borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format.wrapT);
    if(format.compareRefToTex){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, format.compareFunc);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, format.baseLevel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, format.maxLevel); 
    glBindTexture(GL_TEXTURE_2D, 0);
    GL_CHECK_ERRORS;
    return 0;
}

GLTexture2DFormat NormalTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_RGBA16F;
    return format;
}

GLTexture2DFormat ColorTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_RGBA8;
    return format;
}

GLTexture2DFormat HDRColorTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_RGBA16F;
    return format;
}

GLTexture2DFormat VertexTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_RGBA16F;
    return format;
}

GLTexture2DFormat DepthTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_DEPTH;
    format.format = GL_DEPTH;
    return format;
}

};