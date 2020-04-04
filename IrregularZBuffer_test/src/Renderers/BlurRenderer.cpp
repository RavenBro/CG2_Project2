#include "Renderers.h"
void BlurRenderer::Init(int w, int h, GLuint blurT)
    {   
        glGenFramebuffers(1, &framebuffer);
        glGenTextures(1, &tempTex);
        glBindTexture(GL_TEXTURE_2D, tempTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempTex, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
        blurTex = blurT;
        
        glGenFramebuffers(1, &revframebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, revframebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTex, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  
        for (int j=0;j<8;j++)
        {
            float s = sigmas[j];
            float dots[17];
            for(int i = 0; i < 17; i++)
            {
                float p=i/2.0;
                dots[i] = 1.f/std::sqrt(2.f * M_PI * s *s) * std::exp(-(p*p)/(2.f * s * s));
                //std::cout << dots[i] << "\n";
            }    
            weights[j][0]=bases[j];
            for (int i=1;i<8;i++)
            {
                weights[j][i]=1/6.0*(dots[2*i-2]+4*dots[2*i-1]+dots[2*i]);
                weights[j][0]=weights[j][0]-2*weights[j][i];
                //std::cout << weights[j][i] << "\n";
            }
        }
    } 
    void BlurRenderer::Blur(GLuint blurT, int mode)
    {
        blurTex=blurT;
        glBindFramebuffer(GL_FRAMEBUFFER, revframebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTex, 0);
        for (int i=0;i<8;i++)
        {
            cur_weights[i]=weights[mode][i];
        }
        cur_type=mode;
        Render();
    }
    void BlurRenderer::Render()
    {
        auto shader = GetShader();
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
       
        

        shader->StartUseShader();
        shader->SetUniform("isHorizontal", 1);
        shader->SetUniform("extra_blur",extra_blur_quotients[cur_type]);
        auto prog = shader->GetProgram();
        auto loc = glGetUniformLocation(prog, "weight");
        if(loc == -1){
            GL_CHECK_ERRORS;
            throw std::logic_error("No weights found");
        }
        glUniform1fv(loc, 5, cur_weights);
        shader->SetUniform("src_tex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, blurTex);
        DrawObjects();
        glBindFramebuffer(GL_FRAMEBUFFER, revframebuffer);
        shader->SetUniform("src_tex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tempTex);
        shader->SetUniform("isHorizontal", 0);
        shader->SetUniform("extra_blur",extra_blur_quotients[cur_type]);
        prog = shader->GetProgram();
        loc = glGetUniformLocation(prog, "weight");
        if(loc == -1){
            GL_CHECK_ERRORS;
            throw std::logic_error("No weights found");
        }
        glUniform1fv(loc, 5, cur_weights);
        DrawObjects();
        shader->StopUseShader();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    BlurRenderer::~BlurRenderer()
    {
        glDeleteTextures(1, &tempTex);
        glDeleteFramebuffers(1, &framebuffer);
    }