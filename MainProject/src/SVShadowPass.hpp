#pragma once
#include "RenderPass.hpp"
#include <random>
#include <LiteMath.h>
#include "SceneObject.h"
#include "framework/ShaderProgram.h"
#include "framework/HelperGL.h"
#include <glad/glad.h>
#include <cstdio>
#include <vector>

namespace renderer 
{
class SVShadowPass : public RenderPass {
public:
    virtual void OnInit() override;
    virtual void Process() override;
    void Init2(int w,int h);
    void SetShaders(ShaderProgram *first, ShaderProgram *second, ShaderProgram *third);
    GLuint GetShadeTex() {return shadeTex;}
    void Render(float4x4 view, float4x4 projection, float3 campos);
    ~SVShadowPass() {};
private:
    void drawNode(int nodeId, const float4x4& model);
    void draw(const Mesh::Primitive& prim, const float4x4 &model);
    GLuint FrameBuf = 0;
    GLuint depthStencilTex = 0, shadeTex = 0;
    int texWidth,texHeight;
    ShaderProgram  *First,*Second,*Third;
    std::vector<ISceneObject*> occluders;
    std::vector<float3> lights;
    std::vector<float> max_shadow_len;
    struct {
        GLint viewProj = -1, model = -1, lightPos = -1;
        GLint zfar = -1;
    } uniforms;
};
}