#include "Renderers.h"
static float4x4 ortho_matr(float left, float right, float bot, float top, float near, float far)
{
  float4x4 result;
  result.set_row(0, float4(2/(right - left), 0.f, 0.f, -(right + left)/(right - left)));
  result.set_row(1, float4(0.f, 2/(top - bot), 0.f, -(top+bot)/(top - bot)));
  result.set_row(2, float4(0.f, 0.f, -2/(far - near), -(far + near)/(far-near)));
  result.set_row(3, float4(0.f, 0.f, 0.f, 1.f));
  return result;
}
void CascadedShadowMapRenderer::CalcOrthoProjs()
    {
        float4x4 caminv = inverse4x4(transpose4x4(cameraViewTransposed));

      float ar = 600.0 / 800.0;
      float FOV = M_PI/2.0;
      float tanHalfHFOV = tanf(FOV/ 2.0f);
      float tanHalfVFOV = tanf((FOV * ar) / 2.0f);
      for (uint i = 0 ; i < 3 ; i++) 
      {
            float xn = m_cascadeEnd[i]     * tanHalfHFOV;
            float xf = m_cascadeEnd[i + 1] * tanHalfHFOV;
            float yn = m_cascadeEnd[i]     * tanHalfVFOV;
            float yf = m_cascadeEnd[i + 1] * tanHalfVFOV;
 
            float4 frustumCorners[8] = {
                  // Ближняя плоскость
                  float4(xn,   yn, m_cascadeEnd[i], 1.0),
                  float4(-xn,  yn, m_cascadeEnd[i], 1.0),
                  float4(xn,  -yn, m_cascadeEnd[i], 1.0),
                  float4(-xn, -yn, m_cascadeEnd[i], 1.0),
 
                  // Дальняя плоскость
                  float4(xf,   yf, m_cascadeEnd[i + 1], 1.0),
                  float4(-xf,  yf, m_cascadeEnd[i + 1], 1.0),
                  float4(xf,  -yf, m_cascadeEnd[i + 1], 1.0),
                  float4(-xf, -yf, m_cascadeEnd[i + 1], 1.0)
            };
            float4 frustumCornersL[8];
            float minX = 10000;
            float maxX = -10000;
            float minY = 10000;
            float maxY = -10000;
            float minZ = 10000;
            float maxZ = -10000;
            
            for (uint j = 0 ; j < 8 ; j++) 
            {
                // Преобразуем координаты усеченоой пирамиды из пространства камеры в мировое пространство
                
                float4 vW = mul(caminv,frustumCorners[j]);
                // И ещё раз из мирового в пространство света
                frustumCornersL[j] = mul(transpose4x4(lightViewTransposed),vW);
            
                minX = fmin(minX, frustumCornersL[j].x);
                maxX = fmax(maxX, frustumCornersL[j].x);
                minY = fmin(minY, frustumCornersL[j].y);
                maxY = fmax(maxY, frustumCornersL[j].y);
                minZ = fmin(minZ, frustumCornersL[j].z);
                maxZ = fmax(maxZ, frustumCornersL[j].z);
            }
            m_shadowOrthoProjInfo[i]=ortho_matr(minX-1,maxX+1,minY-1,maxY+1,-maxZ-1,-minZ+1);
            
      }
    }
    void CascadedShadowMapRenderer::Render()
    {
        CalcOrthoProjs();
        Attach();
        glClearColor(0.f, 1.f, 0.f, 0.f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        for (int j=0;j<1;j++)
        {
            cascades[j]=mul(m_shadowOrthoProjInfo[j],transpose4x4(lightViewTransposed));
            glClear(GL_DEPTH_BUFFER_BIT);
            auto vsm_s = GetShader();
            vsm_s->StartUseShader();
            for(auto obj : GetObjects())
            {   
                for(unsigned i = 0; i < obj->GetMeshCount(); i++)
                {
                    vsm_s->SetUniform("MVP", mul(cascades[j],obj->GetTransform(i)));
                    obj->Draw(i);
                }
            }
            vsm_s->StopUseShader();
        }
        Unattach(800, 600);
        GL_CHECK_ERRORS;
       
    }
    float *CascadedShadowMapRenderer::GetCasZBorders(float4x4 projection)
    {
        for (int i = 0 ; i < 3 ; i++) 
        {
            float4 vView(0.0f, 0.0f, m_cascadeEnd[i + 1], 1.0f);
            float4 vClip = mul(projection,vView);
            casZborders[i]=-vClip.z;
        }
        return(casZborders);
    }
    void CascadedShadowMapRenderer::Init(int w, int h)
    {
        texWidth=w;
        texHeight=h;
        glGenFramebuffers(1, &FrameBuf);
        
        glGenTextures(1, &DepthTexture);
    glBindTexture(GL_TEXTURE_2D, DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        for (uint i = 0 ; i < CasNum ; i++) 
        {
            glGenTextures(1, &CascadeTex[i]);
            glBindTexture(GL_TEXTURE_2D, CascadeTex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F,  
            texWidth, texHeight, 0, GL_RG, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
 

        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CascadeTex[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, CascadeTex[1], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, CascadeTex[2], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTexture, 0);
        
        GLuint dbuff[]= {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3,dbuff);
        GL_CHECK_ERRORS;
    
        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    
        if (Status != GL_FRAMEBUFFER_COMPLETE) 
        {
            printf("FB error, status: 0x%x\n", Status);
        }

        GL_CHECK_ERRORS;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    CascadedShadowMapRenderer::~CascadedShadowMapRenderer()
    {
        glDeleteTextures(1, &DepthTexture);
        for (uint i = 0 ; i < CasNum ; i++) 
        {
            glDeleteTextures(1, &CascadeTex[i]);
        }
        glDeleteFramebuffers(1, &FrameBuf);
    }
    void CascadedShadowMapRenderer::Attach()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
        glViewport(0, 0, texWidth, texHeight);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CascadeTex[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, CascadeTex[1], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, CascadeTex[2], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTexture, 0);
        
        GLuint dbuff[]= {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3,dbuff);
        GL_CHECK_ERRORS;
    }
    
    void CascadedShadowMapRenderer::Unattach(int width, int height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }