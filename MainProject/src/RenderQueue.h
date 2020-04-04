#pragma once

#include <random>
#include <LiteMath.h>
#include "SceneObject.h"
#include "framework/ShaderProgram.h"
#include "framework/HelperGL.h"
#include <glad/glad.h>

class RenderQueue
{
public:
    RenderQueue() {}

    void AddSceneObject(ISceneObject *sobj)
    {
        if(!sobj)
            throw(std::logic_error("RenderQueue::AddSceneObject(nullptr)"));
        objects.push_back(sobj);
    }

    void DelSceneObject(ISceneObject *sobj)
    {
        if(sobj)
        {
            for(auto iter = objects.begin(); iter != objects.end(); iter++)
            {
                if((*iter) == sobj)
                {
                    objects.erase(iter);
                    return;
                }
            }
            throw std::logic_error("RenderQueue::DelSceneObject - object address not found");
        }
    }

    void SetShaderProg(ShaderProgram *sprog)
    {
        shader = sprog;
    }

    ShaderProgram* GetShaderProg() 
    {
        return shader;
    }

    virtual void Draw() = 0; 

    virtual ~RenderQueue() {}
protected:
    std::vector<ISceneObject*> &GetObjects() { return objects; }
private:
    ShaderProgram *shader = nullptr;
    std::vector<ISceneObject*> objects;
};


class SimpleRenderQueue : public RenderQueue 
{
public:
    void SetProj(const float4x4& p) { projection = p; }
    void SetView(const float4x4& v) { view = v; }
    float4x4 GetProj() { return projection; }
    float4x4 GetView() { return view; }

    virtual void Draw() override
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("projection", GetProj());
        shader->SetUniform("view", GetView());
        for(auto obj : GetObjects())
        {   
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
    }
private:
    float4x4 projection;
    float4x4 view;
};

class RSMRenderQueue : public SimpleRenderQueue 
{
public:
    void SetLightColor(float3 lightC){ 
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("lightColor", lightC);
        shader->StopUseShader();
    }

    virtual void Draw() override
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("projection", GetProj());
        shader->SetUniform("view", GetView());
        for(auto obj : GetObjects())
        {   
            shader->SetUniform("useTex", obj->IsTextured()? 1:0);
            if(!obj->IsTextured()){
                shader->SetUniform("modelColor", obj->GetMaterial().color);
            }
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            { 
                shader->SetUniform("model", obj->GetTransform(i));
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
    }
private:
    float3 lightColor;
};

class DefaultRenderQueue : public SimpleRenderQueue
{
public:
    virtual void Draw() override
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("projection", GetProj());
        shader->SetUniform("view", GetView());
        for(auto obj : GetObjects())
        {   
            shader->SetUniform("use_diffuse_tex", obj->IsTextured()? 1:0);
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            { 
                shader->SetUniform("model", obj->GetTransform(i));
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
    }
};

class TextureRenderer : public RenderQueue
{
public:
    void SetTexture(GLuint texture)
    {
        targetTexture = texture;
    }

    virtual void Draw() override
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, targetTexture);
        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
        shader->StopUseShader();
    }
private:
    GLuint targetTexture = 0;
};

class GBufferRenderQueue : public SimpleRenderQueue
{
public:

    virtual void Draw() override
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("projection", GetProj());
        shader->SetUniform("view", GetView());
        for(auto obj : GetObjects())
        {   
            shader->SetUniform("use_diffuse_tex", (int)obj->IsTextured());
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                auto material = obj->GetMaterial();
                //std::cout << "metallic " << material.metallic << " roughness " << material.roughness << "\n"; 
                shader->SetUniform("metallic", material.metallic);
                shader->SetUniform("roughness", material.roughness);
                shader->SetUniform("reflection", material.reflection);
                shader->SetUniform("color", material.color);
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
    }
};

class LightRenderQueue : public RenderQueue 
{
public:
    LightRenderQueue() : randomFloats(0.f, 1.f) {}
    
    void calculate_SSAO()
    { 
        int s_count=32;
        float scale;
        std::vector<float3> ssaoKernel;
        for (unsigned int i = 0; i < s_count; ++i)
        {
            float3 sample(
                randomFloats(generator) * 2.0 - 1.0, 
                randomFloats(generator) * 2.0 - 1.0, 
                randomFloats(generator)
            );
            sample  = normalize(sample);
            sample *= randomFloats(generator);
            float scale = (float)i / (s_count+0.0);  
            scale   = lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssaoKernel.push_back(sample);  
        }
        std::vector<float3> ssaoNoise;
        for (unsigned int i = 0; i < 16; i++)
        {
            float3 noise(
                randomFloats(generator) * 2.0 - 1.0, 
                randomFloats(generator) * 2.0 - 1.0, 
                0.0f); 
            ssaoNoise.push_back(noise);
        } 
  
        GLfloat xs[s_count],ys[s_count],zs[s_count];
        const GLfloat *xp=xs,*yp=ys,*zp=zs;
        for (int i=0;i<s_count;i++)
        {
            xs[i]=ssaoKernel[i].x;
            ys[i]=ssaoKernel[i].y;
            zs[i]=ssaoKernel[i].z;
    
        }
        //SSAO_s->StartUseShader();
        auto prog = GetShaderProg()->GetProgram();
        auto loc = glGetUniformLocation(prog, "samplesx");
        glUniform1fv(loc,s_count,xp);
        loc = glGetUniformLocation(prog, "samplesy");
        glUniform1fv(loc,s_count,yp);
        loc = glGetUniformLocation(prog, "samplesz");
        glUniform1fv(loc,s_count,zp);  
    }

    virtual void Draw() override 
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("projection", projection);
        shader->SetUniform("normtex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, normTex);
        shader->SetUniform("depthtex", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        shader->SetUniform("diftex", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, vertexTex);
        calculate_SSAO();
        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
        glActiveTexture(GL_TEXTURE0);
        shader->StopUseShader();
    }
    void InitSSAO(GLuint deptht, GLuint normt, GLuint vertext, const float4x4 &proj)
    {
        depthTex = deptht;
        normTex = normt;
        vertexTex = vertext;
        projection = proj;
    }
private:
    float4x4 projection;
    GLuint depthTex = 0, normTex = 0, vertexTex;
    std::default_random_engine generator;
    std::uniform_real_distribution<float> randomFloats;
};

class BlurRenderQueue : public RenderQueue
{
public:
    void Init(int w, int h, GLuint blurT)
    {
        glGenFramebuffers(1, &framebuffer);
        glGenTextures(1, &tempTex);
        glBindTexture(GL_TEXTURE_2D, tempTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
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
        width = w;
        height = h;   
    } 

    void Init(int w, int h, GLuint blurT, GLuint tempT)
    {
        glGenFramebuffers(1, &framebuffer);
        tempTex = tempT;

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempTex, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
        blurTex = blurT;
        
        glGenFramebuffers(1, &revframebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, revframebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTex, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  
        width = w;
        height = h;    
    }

    virtual void Draw() override
    {   
        glViewport(0, 0, width, height);
        auto shader = GetShaderProg();
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        shader->StartUseShader();
        shader->SetUniform("isHorizontal", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, blurTex);
        ScreenDraw();
        glBindFramebuffer(GL_FRAMEBUFFER, revframebuffer);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tempTex);
        shader->SetUniform("isHorizontal", 0);
        ScreenDraw();
        shader->StopUseShader();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~BlurRenderQueue()
    {
        glDeleteTextures(1, &tempTex);
        glDeleteFramebuffers(1, &framebuffer);
    }

    void SetSigma(float sigma)
    {
        float weight[5] {0.f};
        for(int i = 0; i < 5; i++)
        {
            weight[i] = 1.f/std::sqrt(2.f * M_PI * sigma *sigma) * std::exp(-(i*i)/(2.f * sigma * sigma));
            std::cout << weight[i] << "\n";
        }
        auto shader = GetShaderProg();
        shader->StartUseShader();
        auto prog = GetShaderProg()->GetProgram();
        auto loc = glGetUniformLocation(prog, "weight");
        if(loc == -1){
            GL_CHECK_ERRORS;
            throw std::logic_error("No weights found");
        }
        glUniform1fv(loc, 5, weight);
        shader->StopUseShader();
    }

    GLuint GetTemp() { return tempTex; }
private:
    void ScreenDraw(){
        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
    }

    GLuint framebuffer = 0, revframebuffer = 0;
    GLuint tempTex = 0, blurTex = 0; 
    int width = 0, height = 0;
};


class PBRRendererQueue : public RenderQueue
{
public:
    void SetTextures(GLuint diffuseT, GLuint normalT, GLuint vertexT, GLuint materialT)
    {
        diffuseTex = diffuseT;
        normalTex = normalT;
        vertexTex = vertexT;
        materialTex = materialT;
    }

    void SetLightSrc(unsigned n, float3 pos, float3 radiocity)
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("LightPos[" + std::to_string(n) + "]", pos);
        shader->SetUniform("LightColor[" + std::to_string(n) + "]", radiocity);
        shader->StopUseShader();
    }

    void SetGlobalLight(float3 dir, float3 radiocity){
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("GlobalLightDir", dir);
        shader->SetUniform("GlobalLightColor", radiocity);
        shader->StopUseShader();
    }

    void SetRSM(float4x4 viewProj, GLuint depthT, GLuint normT, GLuint posT, GLuint fluxT)
    {
        rsmDepth = depthT;
        rsmFlux = fluxT;
        rsmNorm = normT;
        rsmPos = posT;
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("rsmViewProj", viewProj);
        shader->SetUniform("rsmDepthTex", 5);
        shader->SetUniform("rsmNormTex", 6);
        shader->SetUniform("rsmPosTex", 7);
        shader->SetUniform("rsmFluxTex", 8);

        const int samples_count = 50;
        float samples[samples_count * 2];
        for(int i = 0; i < samples_count; i++){
            float ksi1 = rand() /((float)RAND_MAX);
            float ksi2 = 2*M_PI* rand() /((float)RAND_MAX);
            samples[2*i] = ksi1 * sin(ksi2);
            samples[2*i + 1] = ksi1 * cos(ksi2); 
        }
        auto prog = shader->GetProgram();
        auto loc = glGetUniformLocation(prog, "rsmSamples");
        glUniform2fv(loc, samples_count, samples);
        shader->StopUseShader();
    }

    void SetView(const float4x4 &viewM)
    {
        view = viewM;
    }

    void SetGlobalLightSM(GLuint glsm, const float4x4& lightViewProj){
        glightSM = glsm;
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("GlobalLightVSM", 4);
        shader->SetUniform("GLightSpaceM", lightViewProj);
        shader->StopUseShader();
    }

    void SetInverseCam(const float4x4 &inverseCM){
        inverseCam = inverseCM;
    }

    virtual void Draw() override
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("diffuseTex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTex);
        shader->SetUniform("normalTex", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTex);
        shader->SetUniform("vertexTex", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, vertexTex);
        shader->SetUniform("materialTex", 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, materialTex);
        shader->SetUniform("view", view);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, glightSM);
        shader->SetUniform("InverseCamM", inverseCam);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, rsmDepth);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, rsmNorm);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, rsmPos);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, rsmFlux);

        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
        shader->StopUseShader();
    }
private:
    GLuint diffuseTex = 0, normalTex = 0, vertexTex = 0, materialTex = 0;
    GLuint glightSM = 0; 
    GLuint rsmNorm = 0, rsmFlux = 0, rsmDepth = 0, rsmPos = 0;
    float4x4 view, inverseCam;
};

class SSRRenderQueue : public SimpleRenderQueue 
{
public:
    void SetTextures(GLuint frame, GLuint normal, GLuint depth, GLuint vertex, GLuint material)
    {
        frameTex = frame; 
        normalTex = normal;
        vertexTex = vertex;
        materialTex = material;
        depthTex = depth;
    }

    void SetInvView(const float4x4 &viewM) 
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("InvView", viewM);
        shader->StopUseShader();
    }
    virtual void Draw() override 
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("projection", GetProj());
        shader->SetUniform("frameTex", 0);
        shader->SetUniform("vertexTex", 1);
        shader->SetUniform("depthTex", 2);
        shader->SetUniform("normalTex", 3);
        shader->SetUniform("materialTex", 4);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frameTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, vertexTex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, normalTex);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, materialTex);

        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }

        shader->StopUseShader();
    }

private:

    GLuint frameTex = 0, normalTex = 0, vertexTex = 0, depthTex = 0, materialTex = 0; 

};

class AssembleRenderQueue : public RenderQueue
{
    public:
    void SetTextures(GLuint frame, GLuint reflect, GLuint material)
    {
        frameTex = frame; 
        reflectTex = reflect;
        materialTex = material;
    }

    virtual void Draw() override 
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("frameTex", 0);
        shader->SetUniform("reflectTex", 1);
        shader->SetUniform("materialTex", 2);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frameTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, reflectTex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, materialTex);

        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }

        shader->StopUseShader();
    }

private:
    GLuint frameTex = 0, reflectTex = 0, materialTex = 0; 

};