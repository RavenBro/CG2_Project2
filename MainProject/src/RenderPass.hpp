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

class Renderer;

class RenderPass {
public:
    virtual ~RenderPass() {}

    void Init(Renderer *renderer, Scene *scene, const std::string &name){
        this->renderer = renderer;
        this->scene = scene;
        this->name = name;
        OnInit();
    }
    
    virtual void OnInit() = 0;
    virtual void Process() = 0;

    void SetShader(ShaderProgram *prog) { shader = prog; }
    void SetShader(const std::string &name);

    ShaderProgram *GetShader() { return shader; }

    GLuint BindShader() { shader->StartUseShader(); return shader->GetProgram(); }
    void UnbindShader() { shader->StopUseShader(); }
    
    GLint GetUniformLoc(const std::string &uniform){
        return glGetUniformLocation(shader->GetProgram(), uniform.c_str());
    }
    GLint TryGetUniformLoc(const std::string &uniform);
    
    Renderer *GetRenderer() { return renderer; }
    const Renderer *GetRenderer() const { return renderer; }

    Scene *GetScene() { return scene; }
    const Scene *GetScene() const { return scene; }

    const std::string& GetName() const { return name; }

    bool RegisterTexture(GLuint texture, const std::string &texname);
    GLuint TryGetTexture(const std::string &name);
private:
    std::string name;
    ShaderProgram *shader = nullptr;
    Scene *scene = nullptr;
    Renderer *renderer = nullptr;
};

class TextureDrawer : public RenderPass {
public:

    virtual void OnInit() override;
    virtual void Process() override;
    GLuint GetTarget();
    void SetTarget(GLuint target);
    void Set4Targets(GLuint TL,GLuint TR,GLuint BL,GLuint BR,
                     int TL_t,int TR_t, int BL_t, int BR_t);
    void SetState(int state);
private:
    GLuint targetTex = 0;
    int target_type = 0;
    GLuint TL,TR,BL,BR;
    int TL_type=0,TR_type=0,BL_type=0,BR_type=0;
    PrimitiveMesh* quadMesh;
    void DefaultTypes();
};

class Filter : public RenderPass {
public:
    struct TexInfo {
        GLTexture2DFormat fmt; //format for texture generating
        std::string name; // last part of name to use in Renderer::textures
        GLenum fbTarget; // Framebuffer bind target
        int width, height, level = 0;
    };

    virtual ~Filter(); 

    bool SetTargetTextures(const std::vector<Filter::TexInfo> &textures);
    
    void SetViewport(int4 vp) { viewport = vp; }
    int4 GetViewport() const { return viewport; }
    void Viewport() { glViewport(viewport.x, viewport.y, viewport.z, viewport.w); } 

    GLuint GetFramebuffer() const { return framebuffer; }

    void RenderQuad();

private:
    int4 viewport;
    GLuint framebuffer = 0;
};

class GBufferPass : public RenderPass {
public:
    virtual ~GBufferPass();

    virtual void OnInit() override;
    virtual void Process() override;

    GLuint GetAlbedoTex() const { return colorTex; }
    GLuint GetNormalTex() const { return normalTex; }
    GLuint GetVertexTex() const { return vertexTex; }
    GLuint GetMaterialTex() const { return materialTex; }
     GLuint GetParticlesTex() const { return particlesTex; }
     
private:
    void drawNode(int nodeId, const float4x4 &model);
    void setMaterial(const Material &material);
    void draw(const Mesh::Primitive &prim, const float4x4 &model);

    GLuint framebuffer = 0;
    GLuint colorTex = 0, normalTex = 0, depthTex = 0, vertexTex = 0, 
        materialTex = 0, worldTex = 0, worldNormTex = 0, particlesTex = 0;
    int windowW = -1, windowH = -1;
    struct {
        GLint model, view, projection, useDifTex, difTex, metallic, roughness, reflection, color;
        GLint pbrTex, usePBRTex;
    } uniforms;

    const Scene *targetScene = nullptr;
};

};