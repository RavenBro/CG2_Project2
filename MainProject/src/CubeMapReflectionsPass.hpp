#pragma once

#include "RenderPass.hpp"
#include "SceneObject.h"
#include "framework/ShaderProgram.h"
namespace renderer 
{
class CubeMapReflectionsPass : public RenderPass
{
public:
    virtual void OnInit() override;
    virtual void Process() override;

    void Init2(int w, int h, float3 center);
    void Render();
    ~CubeMapReflectionsPass() {};
    GLuint GetCubeMapTex() {return tex; }
private:
    void drawNode(int nodeId, const float4x4& model);
    void draw(const Mesh::Primitive& prim, const float4x4 &model);
    GLuint FrameBuf = 0, rendBuf = 0;
    GLuint tex = 0, depthTex = 0, resTex = 0;
    GLuint targetTex = 0;
    int texWidth, texHeight;
    ISceneObject *obj;
    ShaderProgram *Drawer;
    float3 center;
    struct {
        GLint viewProj = -1, model = -1, lightPos = -1;
        GLint zfar = -1;
    } uniforms;
};
}