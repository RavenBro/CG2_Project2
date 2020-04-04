#include "Renderers.h"

void HBAORenderer::SetTextures(GLuint DepthT, GLuint RandomT, GLuint MaterialT)
{
    DepthTex = DepthT;
    RandomTex = RandomT;
    MaterialTex = MaterialT;
}
void HBAORenderer::Init(int w,int h)
    {
        texWidth = w;
        texHeight = h;
        glGenFramebuffers(1, &FrameBuffer);

        glGenTextures(1, &HBAOTex);
        glBindTexture(GL_TEXTURE_2D, HBAOTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, HBAOTex, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 

        GL_CHECK_ERRORS;
    }
    void HBAORenderer::Render(float fovRad, float near, float far)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
        glViewport(0, 0, texWidth, texHeight);
        
       #define SRC_WIDTH 800
        #define SRC_HEIGHT 600

        #define RES_RATIO 2
        #define AO_SRC_WIDTH (SRC_WIDTH/RES_RATIO)
        #define AO_HEIGHT (SRC_HEIGHT/RES_RATIO)
        #define AO_RADIUS 0.3
        #define AO_DIRS 6
        #define AO_SAMPLES 3
        #define AO_STRENGTH 2.5;
        #define AO_MAX_RADIUS_PIXELS 50.0

        float FocalLen[2], InvFocalLen[2], UVToViewA[2], UVToViewB[2], LinMAD[2];

        FocalLen[0]      = 1.0f / tanf(fovRad * 0.5f) * ((float)AO_HEIGHT / (float)AO_SRC_WIDTH);
        FocalLen[1]      = 1.0f / tanf(fovRad * 0.5f);
        InvFocalLen[0]   = 1.0f / FocalLen[0];
        InvFocalLen[1]   = 1.0f / FocalLen[1];

        UVToViewA[0] = -2.0f * InvFocalLen[0];
        UVToViewA[1] = -2.0f * InvFocalLen[1];
        UVToViewB[0] =  1.0f * InvFocalLen[0];
        UVToViewB[1] =  1.0f * InvFocalLen[1];

        LinMAD[0] = (near-far)/(2.0f*near*far);
        LinMAD[1] = (near+far)/(2.0f*near*far);

 
        auto shader = GetShader();
        
        shader->StartUseShader();

        float2 tmp=float2(FocalLen[0],FocalLen[1]);
        shader->SetUniform("FocalLen",tmp);
        /*tmp=float2(UVToViewA[0],UVToViewA[1]);
        shader->SetUniform("UVToViewA",tmp);
        tmp=float2(UVToViewB[0],UVToViewB[1]);
        shader->SetUniform("UVToViewB",tmp);
        tmp=float2(LinMAD[0],LinMAD[1]);
        shader->SetUniform("LinMAD",tmp);*/

        shader->SetUniform("texture0", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, DepthTex);
        shader->SetUniform("texture1", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, RandomTex);
        shader->SetUniform("material_tex", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, MaterialTex);
        DrawObjects();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    HBAORenderer::~HBAORenderer()
    {
        glDeleteTextures(1, &HBAOTex);
        glDeleteFramebuffers(1, &FrameBuffer);
    }