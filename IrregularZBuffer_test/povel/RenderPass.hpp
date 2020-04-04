#pragma once

#include <vector>
#include <utility>

#include <LiteMath.h>

#include "framework/ShaderProgram.h"
#include "framework/Primitives.h"
#include "framework/HelperGL.h"

#include "Scenev2.hpp"
#include "GLTexture.hpp"

namespace renderer {

using namespace LiteMath;

class RenderPass {
public:
    virtual ~RenderPass() {}
    virtual void Init(Renderer *renderer, Scene *scene) = 0;
    virtual void Process() = 0;

    void SetShader(ShaderProgram *prog) { shader = prog; }
    ShaderProgram *GetShader() { return shader; }

    GLuint BindShader() { shader->StartUseShader(); return shader->GetProgram(); }
    void UnbindShader() { shader->StopUseShader(); }
    
    GLint GetUniformLoc(const std::string &name){
        return glGetUniformLocation(shader->GetProgram(), name.c_str());
    }
private:
    ShaderProgram *shader = nullptr;
};

class TextureDrawer : public RenderPass {
public:
    void Init(PrimitiveMesh *quad, GLuint texture){
        if(GetShader() == nullptr){
            throw(std::runtime_error("TextureDrawer::Init : shader not set"));
        }
        quadMesh = quad;
        targetTex = texture;
        BindShader();
        auto loc = GetUniformLoc("vsm_tex");
        if(loc == -1){
            throw(std::runtime_error("TextureDrawer::Init : vsm_tex location not found"));
        }
        glUniform1i(loc, 0);
        UnbindShader();
        GL_CHECK_ERRORS;
    }

    virtual void Process() override {
        glViewport(0, 0, 800, 600);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, targetTex);
        BindShader();
        quadMesh->Draw();
        UnbindShader();
    }

private:
    GLuint targetTex = 0;
    PrimitiveMesh* quadMesh;
};

class Filter : public RenderPass {
public:
    virtual ~Filter(); 

    void Init(int width, int height, PrimitiveMesh *quad);
    bool SetOutputTextures(const std::vector<std::pair<GLenum, GLTexture2DFormat>> &formats);
    bool AttachTexture(GLenum target, GLuint texture, int level);
    const std::vector<GLuint>& GetTexures() const;

    virtual void SetUniforms() = 0;
    virtual void Process() override; 

private:
    std::vector<GLuint> textures; 
    GLuint framebuffer = 0;
    PrimitiveMesh *quadMesh = nullptr;
    int viewW, viewH;
};

class GBufferPass : public RenderPass {
public:
    virtual ~GBufferPass();

    void Init(int width, int height, const Scene *scene);
    virtual void Process() override;

    GLuint GetAlbedoTex() const { return colorTex; }
    GLuint GetNormalTex() const { return normalTex; }
    GLuint GetVertexTex() const { return vertexTex; }
    GLuint GetMaterialTex() const { return materialTex; }
private:
    void drawNode(int nodeId, const float4x4 &model);
    void setMaterial(const Material &material);
    void draw(const Mesh::Primitive &prim, const float4x4 &model);

    GLuint framebuffer = 0;
    GLuint colorTex = 0, normalTex = 0, depthTex = 0, vertexTex = 0, materialTex = 0;
    int windowW = -1, windowH = -1;
    struct {
        GLint model, view, projection, useDifTex, difTex, metallic, roughness, reflection, color;
    } uniforms;

    const Scene *targetScene = nullptr;
};

};