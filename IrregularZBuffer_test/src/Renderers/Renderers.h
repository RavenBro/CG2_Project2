#pragma once

#include <random>
#include <LiteMath.h>
#include "../SceneObject.h"
#include "../framework/ShaderProgram.h"
#include "../framework/HelperGL.h"
#include <glad/glad.h>
#include <cstdio>
#include "../OutputController.h"
#include <vector>
class Renderer
{
public:

    void AddObject(ISceneObject *sobj);

    void DelObject(ISceneObject *sobj);


    void SetShader(ShaderProgram *sprog)
    {
        shader = sprog;
    }

    ShaderProgram* GetShader() 
    {
        return shader;
    }
protected:
    std::vector<ISceneObject*> &GetObjects() { return objects; }
    void DrawObjects();
private:
    
    ShaderProgram *shader = nullptr;
    std::vector<ISceneObject*> objects;
};


class SimpleRenderer : public Renderer 
{
public:
    void Render();
    void SetProj(const float4x4& p) { projection = p; }
    void SetView(const float4x4& v) { view = v; }
    float4x4 GetProj() { return projection; }
    float4x4 GetView() { return view; }
    
private:
    float4x4 projection;
    float4x4 view;
};

class DullRenderer : public SimpleRenderer
{
public:
    void Render();
private:
};

class QuadRenderer : public Renderer
{
public:
    void Render();
    void SetTexture(GLuint texture)
    {
        targetTexture = texture;
    }
    GLuint GetTexture()
    {
        return targetTexture;
    }
private:
    GLuint targetTexture = 0;
};

class GBufferRenderer : public SimpleRenderer
{
public:
    ~GBufferRenderer();
    void Render();
    bool Init(int width, int height);
    void SetShadowsTextures(GLuint shadow_tex, GLuint vsm_tex, GLuint cas_near, GLuint cas_middle, GLuint cas_far, GLuint cubemap);
    void SetLights(float4x4 light_ndc, float4x4 *cas_lights, float *casZborders, float3 pointlight);   
    void SetShadows(int type) { shadows_type = type; }
    GLuint GetColorTex() { return colorTex; }
    GLuint GetDepthTex() { return depthTex; }
    GLuint GetNormalTex() { return normalTex; }
    GLuint GetVertexTex() { return vertexTex; }
    GLuint GetNoiseTex() { return noiseTex; }
    GLuint GetShadedTex() { return shadedTex; }
    GLuint GetMaterialTex() { return materialTex; }
    void AttachBuffer();    
    void UnattachBuffer();
    void ClearBuffer();
    void SetBufferRenderState();
    void SetBufferLightingState();
private:
    GLuint framebuffer = 0;
    GLuint colorTex = 0, normalTex = 0, depthTex = 0, vertexTex = 0,
           noiseTex = 0, shadedTex = 0, materialTex = 0, cubeMapShadowsTex = 0;
    int windowW = 0, windowH = 0;  
    GLuint VSMTex = 0, ShadowTex = 0;
    GLuint CSMTex[3] ={0,0,0};
    float4x4 cascadeLights[3];
    float cascadeZborders[3];
    float4x4 LightNDC;
    int shadows_type = 1;
    float3 point_light;
};
class VarianceShadowMapRenderer : public Renderer
{
public:
    void Render();
    void Init(int w, int h);
    ~VarianceShadowMapRenderer();
    void Attach();
    void Unattach(int prev_w, int prev_h);

    void SetLight(const float4x4& l) { light = l; }
    float4x4 GetLight() { return light; }
    GLuint GetVSMTexture() { return VSMTexture; }
private:
    GLuint FrameBuf = 0, DepthTexture = 0, VSMTexture = 0;
    float4x4 light;
    int texWidth, texHeight;
    
};
class CascadedShadowMapRenderer : public Renderer
{
public:
    void Render();
    void Init(int w, int h);
    ~CascadedShadowMapRenderer();
    void Attach();
    void Unattach(int prev_w, int prev_h);
    void CalcOrthoProjs();

    float *GetCasZBorders(float4x4 projection);
    GLuint GetCascade(int num ) const { return(num<CasNum ? CascadeTex[num] : 0); }
    void SetCurCamera(float4x4 ViewTransposed) {  cameraViewTransposed = ViewTransposed;}
    void SetLight(float4x4 lightViewTransposed){ this->lightViewTransposed = lightViewTransposed;}
    float4x4 GetCascadeMatr(int num){ return(cascades[num]);}
    float4x4 *GetLights() { return(cascades);}
    
private:
    float4x4 cascades[3];
    float m_cascadeEnd[4] = {-0.1,-4.0,-10.0,-40.0};
    float4x4 m_shadowOrthoProjInfo[3];
    float4x4 cameraViewTransposed, lightViewTransposed;
    float casZborders[3];
    GLuint FrameBuf = 0;
    const int CasNum = 3;
    GLuint DepthTexture = 0, CascadeTex[3] = {0,0,0};
    int texWidth, texHeight;
};

class BloomRenderer : public Renderer 
{
public:
    void SetTexture (GLuint color);
    GLuint GetBloomTex() { return BloomTex;}
    void Render();
    void Init(int w,int h);
    ~BloomRenderer();
private:
    GLuint colorTex = 0, BloomTex = 0;
    GLuint FrameBuffer;
    int texWidth, texHeight;  
};
class HBAORenderer : public Renderer
{
    public:
    void Render(float fovRad, float near, float far);
    void Init(int w,int h);
    void SetTextures(GLuint DepthT, GLuint RandomT, GLuint MaterialT);
    void DrawHBAO(float fovRad, float near, float far);
    GLuint GetHBAOTex() { return HBAOTex;}
    ~HBAORenderer();
    private:
    GLuint DepthTex,RandomTex,HBAOTex,MaterialTex;
    GLuint FrameBuffer;
    int texWidth, texHeight;  
};
class BlurRenderer : public Renderer
{
public:
    void Render();
    void Init(int w, int h, GLuint blurT);
    void Blur(GLuint blurT, int mode);
    ~BlurRenderer();
    GLuint GetTemp() { return tempTex; }
private:
    GLuint framebuffer = 0, revframebuffer = 0;
    GLuint tempTex = 0, blurTex = 0; 
    float weights[8][8];
    float cur_weights[8];
    float sigmas[8]={0.65,0.65,0.95,0.1,0.1,0.1,0.1,0.1};
    float bases[8]={0.99,0.99,0.99,0,0,0,0,0};
    float extra_blur_quotients[8]={0.1,1.0,0.6,0,0,0,0,0};
    int cur_type=0;
};

class LightRenderer : public Renderer
{
public:
    void Init(int w, int h);
    void Render();
    ~LightRenderer();
    void Attach();
    void Unattach(int prev_w, int prev_h);
    void SetTextures(GLuint diffuseT, GLuint normalT, GLuint vertexT, GLuint materialT);
    void SetLightSrc(unsigned n, float3 pos, float3 radiocity);
    void SetView(const float4x4 &viewM);
    GLuint GetLightTex() { return LightTexture;}
private:
    GLuint diffuseTex = 0, normalTex = 0, vertexTex = 0, materialTex = 0;
    float4x4 view;
    GLuint FrameBuf = 0;
    GLuint LightTexture = 0;
    int texWidth, texHeight;
};

class ScreenReflectionsRenderer : public SimpleRenderer 
{
public:
    void Init(int w, int h);
    void Render(float3 camPos, float4x4 camMatr);
    ~ScreenReflectionsRenderer();
    void Attach();
    void Unattach(int prev_w, int prev_h);
    void SetTextures(GLuint frame, GLuint normal, GLuint depth, GLuint vertex, GLuint material, 
                     GLuint cubemap, GLuint skybox);
    GLuint GetReflectionsTex() { return ReflectionsTexture; }
    GLuint GetRefractionsTex() { return RefractionsTexture; }
private:
    GLuint frameTex = 0, normalTex = 0, vertexTex = 0, depthTex = 0, materialTex = 0, cubeMapTex = 0, skyBoxTex = 0; 
    GLuint FrameBuf = 0;
    GLuint ReflectionsTexture = 0, RefractionsTexture = 0;
    int texWidth, texHeight;  
};

class FinalRenderer : public Renderer
{
public:
    void SetTextures(GLuint frame, GLuint reflect, GLuint material, GLuint bloom,
                     GLuint SSAO, GLuint shade, GLuint color, GLuint refract);
    void Render(OutputController *oc);
private:
    GLuint frameTex = 0, reflectTex = 0, materialTex = 0, bloomTex = 0, SSAOTex = 0, shadeTex = 0, colorTex = 0,
           refractTex = 0; 
};
class SkyBoxRenderer : public Renderer
{
public:
    void Init(int w, int h, GLuint cubeMapTex);
    void Render(float4x4 projection, float4x4 view);
    ~SkyBoxRenderer();
    GLuint GetSkyboxTex() { return SkyBoxResTexture; }
private:
    GLuint FrameBuf = 0;
    GLuint SkyBoxResTexture = 0, SkyBoxCubeMapTexture = 0;
    int texWidth, texHeight;  

};
class CubeMapReflectionsRenderer : public Renderer
{
public:
    void Init(int w, int h, ISceneObject *center);
    void Render();
    ~CubeMapReflectionsRenderer();
    GLuint GetCubeMapTex() {return tex; }
private:
    GLuint FrameBuf = 0;
    GLuint tex = 0, depthTex = 0;
    int texWidth, texHeight;
    ISceneObject *obj;
};
class CubeMapShadowsRenderer : public Renderer 
{
public:
    void Init(int w, int h, float3 pointLight);
    void Render();
    ~CubeMapShadowsRenderer();
    GLuint GetCubeMapTex() {return tex; }
private:
    GLuint FrameBuf = 0;
    GLuint tex = 0, depthTex = 0;
    int texWidth, texHeight;
    float3 pointLight = float3(0,0,0);
};
class ShadowVolumesRenderer : public Renderer
{
public:
    void Init(int w,int h,ISceneObject *quad);
    void SetShaders(ShaderProgram *first, ShaderProgram *second, ShaderProgram *third,
                    ShaderProgram *fourth, ShaderProgram *fifth);
    void SetShadeTex(GLuint shade);
    GLuint GetShadeTex() {return shadeTex;}
    void Render(float4x4 view, float4x4 projection, float3 campos);
    void AddOccluder(ISceneObject *sobj);
    ~ShadowVolumesRenderer();
private:
    GLuint FrameBuf = 0;
    GLuint depthStencilTex = 0, shadeTex = 0;
    int texWidth,texHeight;
    ISceneObject *Quad;
    ShaderProgram *First,*Second, *Third, *Fourth, *Fifth;
    std::vector<ISceneObject*> occluders;
};
class GradientRenderer : public Renderer 
{
    public:
    void SetTexture (GLuint shade, GLuint vertex, GLuint normal);
    GLuint GetGradientTex() { return GradientTex;}
    void Render(float4x4 view,float4x4 proj, float4x4 viewinv);
    void Init(int w,int h);
    ~GradientRenderer();
private:
    GLuint shadeTex = 0, vertexTex = 0, normalTex = 0;
    GLuint  GradientTex = 0;
    GLuint FrameBuffer;
    int texWidth, texHeight; 
    std::vector<float4> shadow_centers;
    std::vector<float> shadow_sizes;
};
class IrregularZBufferRenderer : public Renderer
{
public:
    void Init(int w, int h,ISceneObject *quad);
    void SetTextures(GLuint vertex, GLuint depth);
    void SetShaders(ShaderProgram *dc, ShaderProgram *ir, ShaderProgram *sd);
    GLuint GetShadeTex() { return shadeTex; }
    void Render(float4x4 view, float4x4 proj, float4x4 viewinv, float4x4 light);
    ~IrregularZBufferRenderer();
private:
    GLuint  shadeTex = 0;
    GLuint vertexTex = 0, depthTex = 0;
    GLuint FrameBuf, DataStructure, DepthStructure;
    int texWidth, texHeight;
    ShaderProgram *DataConstructor, *IrregularRasterizer, *ShadeDrawer;
    ISceneObject *Quad;
};