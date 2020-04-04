#pragma once

#include <string>
#include <unordered_map>
#include <LiteMath.h>

#include "framework/Camera.h"
#include "framework/ShaderProgram.h"
#include "framework/LoaderGLTF.h"
#include "framework/HelperGL.h"

#include "Renderers/Renderers.h"
#include "RenderQueue.h"
#include "ShadowMap.h"
class Scene
{
public:
    template <typename T>
    using SceneMap =  std::unordered_map<std::string, std::unique_ptr<T>>;
    OutputController oc;
    void SetViewProj(float4x4 proj, Camera *cam)
    {
        camera = cam;
        projection = proj;
    }
    float3 MoveTank(float HeadAngle,float BodyAngle, float movement);
    void LoadData();
    void SetRenderQueues();
    void Render();
    ~Scene();

private:
    SceneMap<ISceneObject> objects; 
    SceneMap<Mesh> meshes;
    SceneMap<Model> models;
    SceneMap<ShaderProgram> shaders;
    
    float3 lightdir;
    float4x4 projection,light;
    Camera *camera;
    float3 point_light;

    BloomRenderer BloomR;
    BlurRenderer BlurR;
    CascadedShadowMapRenderer CSMR;
    FinalRenderer FinalR;
    GBufferRenderer GBufferR;
    HBAORenderer HBAOR;
    LightRenderer LightR;
    QuadRenderer QuadR;
    ScreenReflectionsRenderer ScrRefR;
    SkyBoxRenderer SkyBoxR;
    CubeMapReflectionsRenderer CubeMapRefR;
    CubeMapShadowsRenderer CubeMapShadowsR;
    ShadowVolumesRenderer ShVolR;
    GradientRenderer GradR;
    IrregularZBufferRenderer IZBufR;

    VarianceShadowMap *vsm;
    CascadedShadowMap *csm;
    VarianceShadowsRenderQueue VSMRQ;
    CascadedShadowsRenderQueue CSMRQ;

   
    enum {SHADOWS_RENDER_DELAY=5};
};