#include "Renderers.h"

static void Show(float4x4 matr) {
      for (int i = 0; i < 4; i++)
      {
          std::cout<<matr.row[i].x<<" ";
          std::cout<<matr.row[i].y<<" ";
          std::cout<<matr.row[i].z<<" ";
          std::cout<<matr.row[i].w<<" ";
          std::cout<<std::endl;
      }
      
  }
void SkyBoxRenderer::Init(int w,int h, GLuint cubeMapTex)
    {
        texWidth = w;
        texHeight = h;
        SkyBoxCubeMapTexture = cubeMapTex;
        glGenFramebuffers(1, &FrameBuf);

        glGenTextures(1, &SkyBoxResTexture);
        glBindTexture(GL_TEXTURE_2D, SkyBoxResTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER,FrameBuf);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SkyBoxResTexture, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 

        GL_CHECK_ERRORS;
    }
    SkyBoxRenderer::~SkyBoxRenderer()
    {
        glDeleteTextures(1, &SkyBoxResTexture);
        glDeleteFramebuffers(1, &FrameBuf);
    }
    void SkyBoxRenderer::Render(float4x4 projection, float4x4 view)
    {
        
        //glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
        //glViewport(0, 0, texWidth, texHeight);
        auto shader = GetShader();
        shader->StartUseShader();
        shader->SetUniform("skybox", 0);
        shader->SetUniform("projection",projection);
        float4x4 viewcpy = float4x4(view);
        viewcpy.row[0].w=0;
        viewcpy.row[1].w=0;
        viewcpy.row[2].w=0;
        viewcpy.row[3].x=0;
        viewcpy.row[3].y=0;
        viewcpy.row[3].z=0;
        shader->SetUniform("view", viewcpy);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, SkyBoxCubeMapTexture);
        DrawObjects();
        shader->StopUseShader();
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }