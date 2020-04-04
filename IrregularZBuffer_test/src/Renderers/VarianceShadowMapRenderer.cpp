#include "Renderers.h"

void VarianceShadowMapRenderer::Render()
{
        Attach();
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        auto vsm_s = GetShader();
        vsm_s->StartUseShader();
        for(auto obj : GetObjects())
        {   
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                vsm_s->SetUniform("MVP", mul(light,obj->GetTransform(i)));
                obj->Draw(i);
            }
        }
        vsm_s->StopUseShader();
        Unattach(800, 600);
        GL_CHECK_ERRORS;
       
    }
    void VarianceShadowMapRenderer::Init(int w, int h)
    {
        texWidth =w;
        texHeight = h;
        glGenFramebuffers(1, &FrameBuf);
        
        glGenTextures(1, &VSMTexture);
        glBindTexture(GL_TEXTURE_2D, VSMTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F,  
            texWidth, texHeight, 0, GL_RG, GL_FLOAT, (void*)0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenTextures(1, &DepthTexture);
        glBindTexture(GL_TEXTURE_2D, DepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,  
            texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
        glBindTexture(GL_TEXTURE_2D, DepthTexture);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DepthTexture, 0);
        glBindTexture(GL_TEXTURE_2D, VSMTexture);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, VSMTexture, 0);
        glReadBuffer(GL_NONE);

        GL_CHECK_ERRORS;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    VarianceShadowMapRenderer::~VarianceShadowMapRenderer()
    {
        glDeleteTextures(1, &VSMTexture);
        glDeleteTextures(1, &DepthTexture);
        glDeleteFramebuffers(1, &FrameBuf);
    }

    void VarianceShadowMapRenderer::Attach()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
        glViewport(0, 0, texWidth, texHeight);
        GL_CHECK_ERRORS;
    }
    
    void VarianceShadowMapRenderer::Unattach(int width, int height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }