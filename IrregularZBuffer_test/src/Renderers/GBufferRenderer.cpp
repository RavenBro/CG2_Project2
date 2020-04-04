#include "Renderers.h"

    void GBufferRenderer::SetShadowsTextures(GLuint shadow_tex, GLuint vsm_tex, GLuint cas_near, GLuint cas_middle,
                                             GLuint cas_far, GLuint cubemap)
    {
        VSMTex = vsm_tex;
        CSMTex[0]=cas_near;
        CSMTex[1]=cas_middle;
        CSMTex[2]=cas_far;
        cubeMapShadowsTex = cubemap;
        ShadowTex = shadow_tex;
    }
    void GBufferRenderer::SetLights(float4x4 light_ndc, float4x4 *cas_lights, float *casZborders,
                                    float3 pointlight)
    {
        LightNDC = light_ndc;
        for (int i=0;i<3;i++)
        {
            cascadeLights[i]=cas_lights[i];
            cascadeZborders[i]=casZborders[i];
        }
        point_light = pointlight;
    }
    void GBufferRenderer::Render()
    {
        auto shader = GetShader();
        shader->StartUseShader();
        shader->SetUniform("pointLightPos",point_light);
        shader->SetUniform("projection", GetProj());
        shader->SetUniform("view", GetView());
        shader->SetUniform("light_NDC",LightNDC);
        shader->SetUniform("near_cascade_light",cascadeLights[0]);
        shader->SetUniform("middle_cascade_light",cascadeLights[1]);
        shader->SetUniform("far_cascade_light",cascadeLights[2]);
        shader->SetUniform("use_shadows",shadows_type);
        auto prog = GetShader()->GetProgram();
        auto loc = glGetUniformLocation(prog, "gCascadeEndClipSpace");
        glUniform1fv(loc,3,cascadeZborders);


        shader->SetUniform("vsm_tex", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, VSMTex);
        shader->SetUniform("nearCascade", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, CSMTex[0]);
        shader->SetUniform("middleCascade", 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, CSMTex[1]);
        shader->SetUniform("farCascade", 4);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, CSMTex[2]);
        shader->SetUniform("cubeMapShadows", 5);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapShadowsTex);
        shader->SetUniform("shadowMap", 6);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, ShadowTex);
        glActiveTexture(GL_TEXTURE0);
        for(auto obj : GetObjects())
        {   
            shader->SetUniform("use_diffuse_tex", (int)obj->IsTextured());
            auto material = obj->GetMaterial();
                shader->SetUniform("metallic", material.metallic);
                shader->SetUniform("roughness", material.roughness);
                shader->SetUniform("reflection", material.reflection);
                shader->SetUniform("transparency", material.transparency);
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
    }
    GBufferRenderer::~GBufferRenderer() 
{
    GLuint textures[] {colorTex, normalTex, depthTex, vertexTex,
     noiseTex, materialTex, shadedTex}; 
    glDeleteTextures(7, textures);
    glDeleteFramebuffers(1, &framebuffer);
}

bool GBufferRenderer::Init(int width, int height)
{
    std::default_random_engine generator;
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); 
    std::vector<LiteMath::float3> ssaoNoise;
    for (unsigned int i = 0; i < 1024; i++)
    {
        LiteMath::float3 noise(
            randomFloats(generator), 
            randomFloats(generator), 
            0.0f); 
        ssaoNoise.push_back(noise);
        //std::cout<<noise.x;
    }  

    windowW = width;
    windowH = height;

    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowW, windowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,   GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &normalTex);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    

    glGenTextures(1, &vertexTex);
    glBindTexture(GL_TEXTURE_2D, vertexTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &noiseTex);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, 32, 32, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
    GL_CHECK_ERRORS;

    glGenTextures(1, &shadedTex);
    glBindTexture(GL_TEXTURE_2D, shadedTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &materialTex);
    glBindTexture(GL_TEXTURE_2D, materialTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    SetBufferRenderState();
    return true;
}

void GBufferRenderer::SetBufferRenderState()
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, vertexTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, shadedTex, 0);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, materialTex, 0);
    GL_CHECK_ERRORS;
    GLuint dbuff[]= {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
         GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
    glDrawBuffers(5, dbuff);
}

void GBufferRenderer::SetBufferLightingState()
{
}

void GBufferRenderer::AttachBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, windowW, windowH);
}

void GBufferRenderer::UnattachBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void GBufferRenderer::ClearBuffer()
{
    AttachBuffer();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}