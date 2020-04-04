#include "Renderers.h"
void CubeMapShadowsRenderer::Init(int w, int h, float3 pointLight)
{
    texHeight = h;
    texWidth = w;
    this->pointLight = pointLight;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    for (int i = 0; i < 6; i++)
    { 
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RG16F, texWidth, texHeight, 0, GL_RG,
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
void CubeMapShadowsRenderer::Render()
{
   
    for (int i = 0; i < 6; i++)
    {
        float4x4 ViewMatrix;
        float4x4 ProjectionMatrix = transpose4x4(projectionMatrixTransposed(
            90, (float)texWidth/(float)texHeight,0.1,100
        ));
        float3 pos =pointLight;
        switch (i)
        {
        case 0:
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
        
        
        for(auto obj : GetObjects())
        {   
            auto m=obj->GetMaterial();
            float4 mat =float4(m.roughness,m.metallic,m.reflection,m.transparency);
            shader->SetUniform("material",mat);
            shader->SetUniform("proj",ProjectionMatrix);
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("MV", mul(ViewMatrix,obj->GetTransform(i)));
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
 CubeMapShadowsRenderer::~CubeMapShadowsRenderer()
{
    GLuint textures[] {tex, depthTex}; 
    glDeleteTextures(2, textures);
    glDeleteFramebuffers(1, &FrameBuf);
}