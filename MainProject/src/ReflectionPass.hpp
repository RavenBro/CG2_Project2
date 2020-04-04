#pragma once

#include "RenderPass.hpp"
#include "BlurPass.hpp"

namespace renderer
{

class ReflectionPass : public Filter {
public:
    void OnInit() override;
    void Process() override;
private:
    struct {
        GLint projection = -1, invView = -1;
        GLint frameTex = -1, vertexTex = -1, depthTex = -1, materialTex = -1, normalTex = -1;
        GLint brdfLuT = -1, albedoTex = -1, normalMat;
    } uniforms;
    GLuint frameTex = 0, vertexTex = 0, depthTex = 0, materialTex = 0, normalTex = 0, brdfTex = 0, albedoTex = 0;
    GLuint maskTex = 0, resTex = 0;

    class ReflectBlur : public RenderPass {
    public:
        virtual void OnInit() override;
        virtual void Process() override;
        void SetTextures(GLuint colorTex, GLuint maskTex, int width, int height, 
            const GLTexture2DFormat &color, const GLTexture2DFormat &mask, int levelCount);
    private:
        struct {
            GLint isHorizontal, pass, colotTex, maskTex;
        } uniforms;

        GLuint srcColor, srcMask, tempColor, tempMask;
        GLuint framebuffer;
        int mipCount, sWidth, sHeight;
    };

    ReflectBlur blur;
};

};