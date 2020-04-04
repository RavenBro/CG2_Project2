#include "Renderers.h"
#include "../framework/Camera.h"
#include "../framework/Game.h"
#include "../framework/Objects3d.h"
#include "../framework/Primitives.h"
#include "../framework/ShaderProgram.h"
#include "../framework/HelperGL.h"
#include "../MyCameras.h"
static void Show_matr(float4x4 matr) {
      for (int i = 0; i < 4; i++)
      {
          std::cout<<matr.row[i].x<<" ";
          std::cout<<matr.row[i].y<<" ";
          std::cout<<matr.row[i].z<<" ";
          std::cout<<matr.row[i].w<<" ";
          std::cout<<std::endl;
      }
      
  }
void CubeMapReflectionsRenderer::Init(int w, int h, ISceneObject *center)
{
    texHeight = h;
    texWidth = w;
    obj = center;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    for (int i = 0; i < 6; i++)
    { 
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA,
        GL_FLOAT, 0);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &FrameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    for (int i = 0; i < 6; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                tex, 0);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    GL_CHECK_ERRORS;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void CubeMapReflectionsRenderer::Render()
{
   
    for (int i = 0; i < 6; i++)
    {
          //if(i==2) continue;
        /*
        GL_TEXTURE_CUBE_MAP_POSITIVE_X  Right       0
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X  Left        1
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y  Top     2
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y  Bottom  3
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z  Back        4
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  Front       5
        */
        
        /* Выставляется камера для нужной грани */
        float4x4 ViewMatrix;
        float4x4 ProjectionMatrix = transpose4x4(projectionMatrixTransposed(
            90, (float)texWidth/(float)texHeight,0.3,100
        ));
        float3 pos =float3(0,0.5,0);
        switch (i)
        {
        case 0:
            /*ViewMatrix=transpose4x4(lookAtTransposed(
                float3(-1,0,0),float3(0.0,0.5,0.0),float3(0,-1,0)
            ));*/
            ViewMatrix=transpose4x4(lookAtTransposed(
                pos,pos+float3(1.0,0.0,0.0),float3(0,-1,0)
            ));
            break;
        case 1:
            ViewMatrix=transpose4x4(lookAtTransposed(
                pos,pos+float3(-1,0,0),float3(0,-1,0)
            ));
            break;
        case 2:
            ViewMatrix=transpose4x4(lookAtTransposed(
                pos,pos+float3(0,1,0),float3(0,0,-1)
            ));
            break;
        case 3:
            ViewMatrix=transpose4x4(lookAtTransposed(
               pos,pos+float3(0,-1,0),float3(0,0,-1)
            ));
            break;
        case 4:
            ViewMatrix=transpose4x4(lookAtTransposed(
                pos,pos+float3(0,0,1),float3(0,-1,0)
            ));
            break;
        case 5:
            ViewMatrix=transpose4x4(lookAtTransposed(
                pos,pos+float3(0,0,-1),float3(0,-1,0)
            ));
        default:
            break;
        };
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
        glViewport(0, 0, texWidth, texHeight);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
        glDrawBuffer(GL_COLOR_ATTACHMENT0 +i);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i , 
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex, 0);

        GL_CHECK_ERRORS;
        auto shader = GetShader();
        shader->StartUseShader();
        
        shader->SetUniform("projection", ProjectionMatrix);
        
        shader->SetUniform("view", ViewMatrix);
        
        for(auto obj : GetObjects())
        {   
            shader->SetUniform("use_diffuse_tex", (int)obj->IsTextured());
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
 CubeMapReflectionsRenderer::~CubeMapReflectionsRenderer()
{
    GLuint textures[] {tex, depthTex}; 
    glDeleteTextures(2, textures);
    glDeleteFramebuffers(1, &FrameBuf);
}