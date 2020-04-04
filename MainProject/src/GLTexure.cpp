#include "GLTexture.hpp"

#include <LiteMath.h>
#include <vector>
#include <random>

namespace renderer 
{

using namespace LiteMath;

GLuint GenerateTexture2D(const GLTexture2DFormat &format, int width, int height, int level, void *data){
    GLuint texture = 0;
    glGenTextures(1, &texture);
    GL_CHECK_ERRORS;
    glBindTexture(GL_TEXTURE_2D, texture);
    GL_CHECK_ERRORS;
    glTexImage2D(GL_TEXTURE_2D, level, format.internalFmt, width, height, 0, format.format, format.pixelType, data);
    GL_CHECK_ERRORS;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, format.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, format.magFilter);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, format.borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format.wrapT);
    if(format.compareRefToTex){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, format.compareFunc);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, format.baseLevel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, format.maxLevel); 
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL_CHECK_ERRORS;
    return texture;
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
    format.wrapS = format.wrapT = GL_CLAMP_TO_EDGE;
    return format;
}

GLTexture2DFormat HDRColorTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_RGBA16F;
    format.format = GL_RGBA;
    for(int i = 0; i < 4; i++) format.borderColor[i] = 0.f;
    format.wrapS = format.wrapT = GL_CLAMP_TO_BORDER;
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
    format.internalFmt = GL_DEPTH_COMPONENT;
    format.format = GL_DEPTH_COMPONENT;
    return format;
}

GLTexture2DFormat AOTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_RGBA16;
    format.format = GL_RGBA;
    return format;
}

GLTexture2DFormat SMTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_DEPTH_COMPONENT24;
    format.format = GL_DEPTH_COMPONENT;
    format.compareRefToTex = true;
    format.compareFunc = GL_LEQUAL;
    return format;
}

GLTexture2DFormat VSMTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_RG32F;
    format.format = GL_RG;
    format.wrapS = format.wrapT = GL_CLAMP_TO_EDGE;
    return format;
}

GLTexture2DFormat WorldTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_RGB16F;
    format.format = GL_RGB;
    format.wrapS = format.wrapT = GL_CLAMP_TO_EDGE;
    return format;
}

GLTexture2DFormat BRDFTexFormat(){
    GLTexture2DFormat format;
    format.minFilter = format.magFilter = GL_LINEAR;
    format.internalFmt = GL_RG16F;
    format.format = GL_RG;
    format.wrapS = format.wrapT = GL_CLAMP_TO_EDGE;
    return format;
}

GLTexture2DFormat ReflectTexFormat(){
    GLTexture2DFormat format;
    format.magFilter = GL_LINEAR;
    format.minFilter = GL_LINEAR_MIPMAP_LINEAR;
    format.internalFmt = GL_RGBA16F;
    format.format = GL_RGBA;
    format.wrapS = format.wrapT = GL_CLAMP_TO_EDGE;
    //format.maxLevel = 5;
    return format;
}

GLTexture2DFormat ReflectMaskFormat(){
    GLTexture2DFormat format;
    format.magFilter = GL_LINEAR;
    format.minFilter = GL_LINEAR_MIPMAP_LINEAR;
    format.internalFmt = GL_R16F;
    format.format = GL_RED;
    format.wrapS = format.wrapT = GL_CLAMP_TO_EDGE;
    //format.maxLevel = 5;
    return format;
}

GLuint GenerateNoiseTexture2D(int width, int height){
    std::vector<float3> noise;
    //noise.reserve(3 * width * height);
    std::default_random_engine generator;
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); 
    for(int i = 0; i < width * height; i++){
        noise.push_back(float3(randomFloats(generator), randomFloats(generator), randomFloats(generator)));
        //noise.push_back(randomFloats(generator));
        //noise.push_back(randomFloats(generator));
    }


    GLTexture2DFormat fmt = HDRColorTexFormat();
    fmt.internalFmt = GL_RGB16;
    fmt.format = GL_RGB;
    fmt.minFilter = fmt.magFilter = GL_LINEAR;
    //fmt.
    return GenerateTexture2D(fmt, width, height, 0, (void*)noise.data());
}

};