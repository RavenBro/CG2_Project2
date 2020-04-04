#pragma once
#include "RenderPass.hpp"
namespace renderer 
{
class CascadedShadowPass : public RenderPass
{
public:
    float *GetCasZBorders(float4x4 projection);
    void CalcOrthoProjs();
    float4x4 ortho_matr(float left, float right, float bot, float top, float near, float far);
    void Init2(int w,int h);
    void drawNode(int nodeId, const float4x4& model);
    void draw(const Mesh::Primitive& prim, const float4x4 &model);
    virtual void OnInit() override;
    virtual void Process() override;
    float4x4 GetCascadeMatr(int num)
    {
        return(cascades[num]);
    }
    float4x4 *GetLights()
    {
        return(cascades);
    }
    void SetCurCamera(float4x4 ViewTransposed) 
    {  
        cameraViewTransposed = ViewTransposed;
    }
    void SetLight(float4x4 lightViewTransposed)
    {
        this->lightViewTransposed = lightViewTransposed;
    }
    GLuint GetCascade(int n) { return cascadeTex[n];}
private:
    GLuint FrameBuf = 0, depthTex = 0;
    GLuint cascadeTex[3];
    float4x4 cascades[3];
    float m_cascadeEnd[4] = {-0.1,-2,-8.0,-40.0};
    float4x4 m_shadowOrthoProjInfo[3];
    float4x4 cameraViewTransposed, lightViewTransposed;
    float casZborders[3];
    int texWidth,texHeight;
    struct {
        GLint MVP = -1;
    } uniforms;
};
}