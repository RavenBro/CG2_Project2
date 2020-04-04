#pragma once

#include <random>
#include <LiteMath.h>
#include "SceneObject.h"
#include "framework/ShaderProgram.h"
#include "framework/HelperGL.h"
#include <glad/glad.h>
#include "ShadowMap.h"
#include <cstdio>
static float4x4 ortho_matr(float left, float right, float bot, float top, float near, float far)
{
  float4x4 result;
  result.set_row(0, float4(2/(right - left), 0.f, 0.f, -(right + left)/(right - left)));
  result.set_row(1, float4(0.f, 2/(top - bot), 0.f, -(top+bot)/(top - bot)));
  result.set_row(2, float4(0.f, 0.f, -2/(far - near), -(far + near)/(far-near)));
  result.set_row(3, float4(0.f, 0.f, 0.f, 1.f));
  return result;
}
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
    GLuint GetTexture()
    {
        return targetTexture;
    }
    virtual void Draw() override
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("vsm_tex", 0);
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
    void SetShadowsTextures(GLuint vsm_tex, GLuint cas_near, GLuint cas_middle, GLuint cas_far)
    {
        VSMTex = vsm_tex;
        CSMTex[0]=cas_near;
        CSMTex[1]=cas_middle;
        CSMTex[2]=cas_far;
    }
    void SetLights(float4x4 light_ndc, float4x4 *cas_lights, float *casZborders)
    {
        LightNDC = light_ndc;
        for (int i=0;i<3;i++)
        {
            cascadeLights[i]=cas_lights[i];
            cascadeZborders[i]=casZborders[i];
        }
    }
    void SetShadows(int type)
    {
        shadows_type = type;
    }
    virtual void Draw() override
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("projection", GetProj());
        shader->SetUniform("view", GetView());
        shader->SetUniform("light_NDC",LightNDC);
        shader->SetUniform("near_cascade_light",cascadeLights[0]);
        shader->SetUniform("middle_cascade_light",cascadeLights[1]);
        shader->SetUniform("far_cascade_light",cascadeLights[2]);
        shader->SetUniform("use_shadows",shadows_type);
        auto prog = GetShaderProg()->GetProgram();
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
        glActiveTexture(GL_TEXTURE0);
        for(auto obj : GetObjects())
        {   
            shader->SetUniform("use_diffuse_tex", (int)obj->IsTextured());
            auto material = obj->GetMaterial();
                //std::cout << "metallic " << material.metallic << " roughness " << material.roughness << "\n"; 
                shader->SetUniform("metallic", material.metallic);
                shader->SetUniform("roughness", material.roughness);
                shader->SetUniform("reflection", material.reflection);
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
    GLuint VSMTex = 0;
    GLuint CSMTex[3] ={0,0,0};
    float4x4 cascadeLights[3];
    float cascadeZborders[3];
    float4x4 LightNDC;
    int shadows_type = 1;
};
class VarianceShadowsRenderQueue : public RenderQueue
{
public:
    void SetLight(const float4x4& l) { light = l; }
    float4x4 GetLight() { return light; }
    void setVSM(VarianceShadowMap *varsm)
    {
        vsm = varsm;
    }
    virtual void Draw() override
    {
        vsm->BindTarget();
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        auto vsm_s = GetShaderProg();
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
        vsm->Unbind(800, 600);
        GL_CHECK_ERRORS;
       
    }
private:
    VarianceShadowMap *vsm;
    float4x4 light;
    
};
class CascadedShadowsRenderQueue : public RenderQueue
{
public:
    
    void setCSM(CascadedShadowMap *cassm)
    {
        csm = cassm;
    }
    void SetCurCamera(float4x4 ViewTransposed) 
    {  
        cameraViewTransposed = ViewTransposed;
    }
    void SetLight(float4x4 lightViewTransposed)
    {
        this->lightViewTransposed = lightViewTransposed;
    }
    void CalcOrthoProjs()
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
    virtual void Draw() override
    {
        CalcOrthoProjs();
        csm->BindTarget();
        for (int j=0;j<3;j++)
        {
            cascades[j]=mul(m_shadowOrthoProjInfo[j],transpose4x4(lightViewTransposed));
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
             GL_TEXTURE_2D, csm->GetCascade(j), 0);
            glClearColor(0.f, 0.f, 0.f, 0.f);
            glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
            auto vsm_s = GetShaderProg();
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
        csm->Unbind(800, 600);
        GL_CHECK_ERRORS;
       
    }
    float4x4 GetCascadeMatr(int num)
    {
        return(cascades[num]);
    }
    float4x4 *GetLights()
    {
        return(cascades);
    }
    float *GetCasZBorders(float4x4 projection)
    {
        for (int i = 0 ; i < 3 ; i++) 
        {
            float4 vView(0.0f, 0.0f, m_cascadeEnd[i + 1], 1.0f);
            float4 vClip = mul(projection,vView);
            casZborders[i]=-vClip.z;
        }
        //printf("borders %f %f %f \n",casZborders[0],casZborders[1],casZborders[2]);
        return(casZborders);
    }
private:
    CascadedShadowMap *csm;
    float4x4 cascades[3];
    float m_cascadeEnd[4] = {-0.1,-4.0,-10.0,-40.0};
    float4x4 m_shadowOrthoProjInfo[3];
    float4x4 cameraViewTransposed, lightViewTransposed;
    float casZborders[3];
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
                randomFloats(generator), 
                randomFloats(generator), 
                randomFloats(generator)
            );
            sample  = normalize(sample);
            sample *= randomFloats(generator);
            float scale = (float)i / (s_count+0.0);  
            scale   = lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssaoKernel.push_back(sample);  
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
        shader->SetUniform("noisetex", 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, noiseTex);
        shader->SetUniform("colortex", 4);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, colorTex);
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
    void InitSSAO(GLuint deptht, GLuint normt, GLuint vertext, GLuint noiset, GLuint colort, const float4x4 &proj)
    {
        depthTex = deptht;
        normTex = normt;
        vertexTex = vertext;
        projection = proj;
        noiseTex = noiset;
        colorTex = colort;
    }
private:
    float4x4 projection;
    GLuint depthTex = 0, normTex = 0, vertexTex = 0, noiseTex = 0, colorTex = 0;
    std::default_random_engine generator;
    std::uniform_real_distribution<float> randomFloats;
};
class BloomRenderQueue : public RenderQueue 
{
public:
    
    virtual void Draw() override 
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("vsm_tex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
        glActiveTexture(GL_TEXTURE0);
        shader->StopUseShader();
    }
    void SetTexture (GLuint color)
    {
        colorTex = color;
    }
private:
    GLuint colorTex = 0;
};
class HBAORenderQueue : public RenderQueue
{
    public:
    void Init(int w,int h, GLuint DepthT, GLuint RandomT)
    {
        DepthTex = DepthT;
        RandomTex = RandomT;
        glGenFramebuffers(1, &FrameBuffer);

        glGenTextures(1, &HBAOTex);
        glBindTexture(GL_TEXTURE_2D, HBAOTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, HBAOTex, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 

        GL_CHECK_ERRORS;
    }
    virtual void Draw() override
    {
        DrawHBAO(M_PI/2,0.1,100);
    }
    void DrawHBAO(float fovRad, float near, float far)
    {

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

 
        auto shader = GetShaderProg();
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

        shader->StartUseShader();

        float2 tmp=float2(FocalLen[0],FocalLen[1]);
        shader->SetUniform("FocalLen",tmp);
        tmp=float2(UVToViewA[0],UVToViewA[1]);
        shader->SetUniform("UVToViewA",tmp);
        tmp=float2(UVToViewB[0],UVToViewB[1]);
        shader->SetUniform("UVToViewB",tmp);
        tmp=float2(LinMAD[0],LinMAD[1]);
        shader->SetUniform("LinMAD",tmp);
        /*tmp=float2((float)SRC_WIDTH, (float)SRC_HEIGHT);
        shader->SetUniform("AORes",tmp);
        tmp=float2(1.0f/(float)SRC_WIDTH, 1.0f/(float)SRC_HEIGHT);
        shader->SetUniform("InvAORes",tmp);
        tmp=float2((float)SRC_WIDTH/(float)4, (float)SRC_HEIGHT/(float)4);
        shader->SetUniform("NoiseScale", tmp);
        shader->SetUniform("R",(float)AO_RADIUS);
        shader->SetUniform("R2",(float)AO_RADIUS*AO_RADIUS);
        shader->SetUniform("R2",-1.0f / (AO_RADIUS*AO_RADIUS));
        shader->SetUniform("MaxRadiusPixels",(float)AO_MAX_RADIUS_PIXELS);*/
        

        shader->SetUniform("texture0", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, DepthTex);
        shader->SetUniform("texture1", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, RandomTex);
        ScreenDraw();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    GLuint GetHBAOTex() { return HBAOTex;}
    virtual ~HBAORenderQueue()
    {
        glDeleteTextures(1, &HBAOTex);
        glDeleteFramebuffers(1, &FrameBuffer);
    }
    private:
    void ScreenDraw()
    {
        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
    }
    GLuint DepthTex,RandomTex,HBAOTex;
    GLuint FrameBuffer;
};
class BlurRenderQueue : public RenderQueue
{
public:
    void Init(int w, int h, GLuint blurT)
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
                std::cout << dots[i] << "\n";
            }    
            weights[j][0]=bases[j];
            for (int i=1;i<8;i++)
            {
                weights[j][i]=1/6.0*(dots[2*i-2]+4*dots[2*i-1]+dots[2*i]);
                weights[j][0]=weights[j][0]-2*weights[j][i];
                std::cout << weights[j][i] << "\n";
            }
        }
    } 
    void Blur(GLuint blurT, int mode)
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
        Draw();
    }
    virtual void Draw() override
    {
        auto shader = GetShaderProg();
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
        ScreenDraw();
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
        ScreenDraw();
        shader->StopUseShader();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual ~BlurRenderQueue()
    {
        glDeleteTextures(1, &tempTex);
        glDeleteFramebuffers(1, &framebuffer);
    }

    GLuint GetTemp() { return tempTex; }
private:
    void ScreenDraw()
    {
        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
    }

    GLuint framebuffer = 0, revframebuffer = 0;
    GLuint tempTex = 0, blurTex = 0; 
    float weights[8][8];
    float cur_weights[8];
    float sigmas[8]={0.65,0.65,0.1,0.1,0.1,0.1,0.1,0.1};
    float bases[8]={0.99,0.99,0,0,0,0,0,0};
    float extra_blur_quotients[8]={0.1,1.0,0,0,0,0,0,0};
    int cur_type=0;
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

    void SetView(const float4x4 &viewM)
    {
        view = viewM;
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
    float4x4 view;
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
    void SetTextures(GLuint frame, GLuint reflect, GLuint material,
                     GLuint bloom, GLuint SSAO, GLuint shade)
    {
        frameTex = frame; 
        reflectTex = reflect;
        materialTex = material;
        bloomTex = bloom;
        SSAOTex = SSAO;
        shadeTex = shade;
    }

    virtual void Draw() override 
    {
        auto shader = GetShaderProg();
        shader->StartUseShader();
        shader->SetUniform("frameTex", 0);
        shader->SetUniform("reflectTex", 1);
        shader->SetUniform("materialTex", 2);
        shader->SetUniform("bloomTex", 5);
        shader->SetUniform("SSAOTex", 6);
        shader->SetUniform("shadeTex", 7);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frameTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, reflectTex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, materialTex);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, bloomTex);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, SSAOTex);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, shadeTex);
        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }

        shader->StopUseShader();
    }

private:
    GLuint frameTex = 0, reflectTex = 0, materialTex = 0,
    bloomTex = 0, SSAOTex = 0, shadeTex = 0; 

};