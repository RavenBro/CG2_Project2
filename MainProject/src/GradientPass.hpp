#pragma once
#include "RenderPass.hpp"

namespace renderer 
{
class GradientPass : public RenderPass
{
public:
    void SetTexture (GLuint shade, GLuint vertex, GLuint normal);
    GLuint GetGradientTex() { return GradientTex;}
    void Render(float4x4 view,float4x4 proj, float4x4 viewinv);
    void Init2(int w,int h);
    virtual void OnInit() override;
    virtual void Process() override;
private:
    GLuint shadeTex = 0, vertexTex = 0, normalTex = 0;
    GLuint  GradientTex = 0;
    GLuint FrameBuffer;
    int texWidth, texHeight; 
    std::vector<float4> shadow_centers;
    std::vector<float> shadow_sizes;
};
}