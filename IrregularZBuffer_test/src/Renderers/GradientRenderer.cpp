#include "Renderers.h"

void GradientRenderer::Init(int w,int h)
    {
        texWidth = w;
        texHeight = h;
        glGenFramebuffers(1, &FrameBuffer);

        glGenTextures(1, &GradientTex);
        glBindTexture(GL_TEXTURE_2D, GradientTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
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
    GradientRenderer::~GradientRenderer()
    {
        glDeleteTextures(1, &GradientTex);
        glDeleteFramebuffers(1, &FrameBuffer);
    }
    void GradientRenderer::SetTexture(GLuint shade, GLuint vertex, GLuint normal)
    {
        shadeTex = shade;   
        vertexTex = vertex;
        normalTex = normal;
    }
    void GradientRenderer::Render(float4x4 view,float4x4 proj, float4x4 viewinv)
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
        shadow_centers.push_back(float4(-0.85,0.001,6.55,1.0));
        shadow_centers.push_back(float4(3.15,0.001,6.55,1.0));
        shadow_centers.push_back(float4(-4.3,0.001,8.5,1.0));
        shadow_sizes.resize(0);
        shadow_sizes.push_back(5);
        shadow_sizes.push_back(5);
        shadow_sizes.push_back(2);
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
        DrawObjects();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }