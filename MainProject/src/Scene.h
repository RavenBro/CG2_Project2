#pragma once

#include <string>
#include <unordered_map>
#include <LiteMath.h>

#include "framework/Camera.h"
#include "framework/ShaderProgram.h"
#include "framework/LoaderGLTF.h"
#include "framework/HelperGL.h"

#include "GBuffer.h"
#include "RenderQueue.h"
#include "ShadowMap.h"

class Scene
{
public:
    template <typename T>
    using SceneMap =  std::unordered_map<std::string, std::unique_ptr<T>>;
    
    void SetViewProj(float4x4 proj, Camera *cam)
    {
        camera = cam;
        projection = proj;
    }

    void LoadData();
    void CreateObjects();
    void Render();

private:
    SceneMap<ISceneObject> objects; 
    SceneMap<Mesh> meshes;
    SceneMap<Model> models;
    SceneMap<ShaderProgram> shaders;
    
    float4x4 projection;
    Camera *camera;

    DefaultRenderQueue defRQ;
    TextureRenderer texRQ;
    SimpleRenderQueue skyboxRQ;
    SimpleRenderQueue vsmRQ;
    LightRenderQueue lightRQ;
    GBufferRenderQueue gbuffRQ;
    BlurRenderQueue blurRQ, vsmBlurRQ;
    
    PBRRendererQueue pbrRQ;
    SSRRenderQueue ssrRQ;
    AssembleRenderQueue asmRQ;

    GBuffer gbuff;
    PostBuffer pbuff;
    ReflectBuff rbuff;

    std::unique_ptr<VarianceShadowMap> globLightSM;
    ReflectiveShadowMap refSM; 
    RSMRenderQueue rsmRQ;
};