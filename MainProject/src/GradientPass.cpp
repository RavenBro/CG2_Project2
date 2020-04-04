#include "GradientPass.hpp"
#include "Renderer.hpp"
namespace renderer
{
void GradientPass::Init2(int w,int h)
    {
        texWidth = w;
        texHeight = h;
        glGenFramebuffers(1, &FrameBuffer);

        glGenTextures(1, &GradientTex);
        glBindTexture(GL_TEXTURE_2D, GradientTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, texWidth, texHeight, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GradientTex, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
        GL_CHECK_ERRORS;
    }
    void GradientPass::SetTexture(GLuint shade, GLuint vertex, GLuint normal)
    {
        shadeTex = shade;   
        vertexTex = vertex;
        normalTex = normal;
    }
    void GradientPass::Render(float4x4 view,float4x4 proj, float4x4 viewinv)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
        glViewport(0, 0, texWidth, texHeight);
        auto shader = GetShader();
        
        shader->StartUseShader();
        shader->SetUniform("shade", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadeTex);

        shader->SetUniform("vertex", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, vertexTex);

        //shader->SetUniform("normal", 2);
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, normalTex);
        shadow_centers.resize(0);
        //shadow_centers.push_back(float4(-1.14,0.001,0.05,1.0));
        shadow_centers.push_back(float4(3.3855, 0.583243, -2.67684,1.0));
        shadow_centers.push_back(float4(3.72156, 0.547311, -2.67726,1.0));
        shadow_centers.push_back(float4(3.72929, 0.558288, -2.33644,1.0));
        shadow_centers.push_back(float4(3.57212, 0.584791, -2.18863,1.0));
        shadow_centers.push_back(float4(3.22601, 0.548419, -2.51386,1.0));
        //shadow_centers.push_back(float4(-4.3,0.001,8.5,1.0));
        shadow_sizes.resize(0);
        shadow_sizes.push_back(1);
        shadow_sizes.push_back(1);
        shadow_sizes.push_back(1);
        shadow_sizes.push_back(1);
        shadow_sizes.push_back(1);
        //shadow_sizes.push_back(2.5);
        //shadow_sizes.push_back(2);
        //printf("%d",(int)shadow_centers.size());
        shader->SetUniform("shadow_centers",(int)shadow_centers.size());
        for (int i=0;i<shadow_centers.size();i++)
        {
            float4 res = mul(view,shadow_centers[i]);
            shader->SetUniform("shadow_centers_camera[" + std::to_string(i) + "]",
                               float3(res.x,res.y,res.z));
        res = mul(proj,res);
        res = res/res.w;
        shader->SetUniform("shadow_centers_screen[" + std::to_string(i) + "]",
                            float3(0.5*res.x+0.5,0.5*res.y+0.5,0));
        shader->SetUniform("shadow_sizes[" + std::to_string(i) + "]",shadow_sizes[i]);
        }
        //printf("<%f %f %f %f>",res.x,res.y,res.z,res.w);
        GetRenderer()->GetScreenQuad()->Draw();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void GradientPass::OnInit()
    {
        Init2(800,600);
        SetShader(GetRenderer()->GetShader("gradient_s"));
    if(GetShader() == nullptr){
        throw(std::runtime_error("Gradient::Init : shader not set"));
    }
    if(!RegisterTexture(GradientTex, "Grad")){
        throw(std::runtime_error("AddTextureName"));
    }
        shadeTex = GetRenderer()->GetTexture("PenumbraShadows/Shade");
        vertexTex = GetRenderer()->GetTexture("GBuffer/Vertex");
    }
    void GradientPass::Process()
    {
        Render(GetScene()->sceneCamera.view,GetScene()->sceneCamera.projection,
               inverse4x4(GetScene()->sceneCamera.view));
    }
}