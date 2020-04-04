#pragma once

#include <unordered_map>
#include <memory>

#include "Scenev2.hpp"
#include "RenderPass.hpp" 
#include "framework/Primitives.h"

#include "PBRPass.hpp"
#include "AssemblePass.hpp"
#include "ReflectionPass.hpp"
#include "AOPass.hpp"
#include "SMPass.hpp"
#include "VSMPass.hpp"
#include "CMShadowPass.hpp"
#include "SkyboxPass.hpp"
#include "RSMPass.hpp"
#include "IndirectLightPass.hpp"

#include "GenCubemapPass.hpp"
#include "IBLcubemapPass.hpp" 
#include "BRDFIntegrPass.hpp"
#include "IBLSpecPass.hpp"

#include "CaptureScene.hpp"


#include "SVShadowPass.hpp"
#include "PenumbraShadowPass.hpp"
#include "GradientPass.hpp"
#include "CubeMapReflectionsPass.hpp"
#include "CascadedShadowPass.hpp"
#include "ParticlesPass.hpp"
namespace renderer
{

struct Config {
    bool useReflections = false;
    bool useAO = false;
    bool useShadows = false;
    bool useBloom = false;
    bool useIndirectLighting = false;
    bool useIBL = false;
    bool tex_switch = true;//true - переключение текстур, false - переключение камер
};

class Renderer{
public:
    void Init(Scene *scene, int4 viewport);
    void Draw();

    ShaderProgram *GetShader(const std::string &name);
    GLuint GetTexture(const std::string &name);
    bool AddTextureName(const std::string &name, GLuint tex);

    int4 GetScreenViewport() const { return screenViewport; }
    void SetViewport(int4 vp) {screenViewport = vp;}
    PrimitiveMesh *GetScreenQuad() const { return quad.get(); }
    PrimitiveMesh *GetSkyboxCube() const { return skyboxCube.get(); }
    GLuint GetEmptyCubemapTex() const { return simpleCubemap; } 
    void SetConf(const Config &c) { config = c; }
    const Config &GetConf() const { return config; }


    void LoadShaders();

    const Scene* targetScene = nullptr;
    
    int4 screenViewport;

    Config config;

    SMPass shadowMaps;
    VSMPass vsMaps;
    CMShadowPass cmShadows;
    RSMPass rsm;


    GBufferPass gbuffer;
    SkyboxPass skybox;
    PBRPass pbr;
    IndirectLightPass indirLight;
    AssemblePass assembler;
    ReflectionPass reflects;
    AOPass ambientOcclusion;
    TextureDrawer texDraw;
    
    GenCubemapPass envCubemapTransf;
    IBLcubemapPass IBLcubemapGen;
    BRDFIntegrPass integrateBRDF;
    IBLSpecPass integrateSpec;
    
    CaptureScene captureScene;

    SVShadowPass svShadows;
    PenumbraShadowPass penumbraShadows;
    GradientPass gradient;
    CascadedShadowPass CSShadows;
    ParticlesPass part;
    CubeMapReflectionsPass CMRPass;

    GLuint simpleCubemap = 0;
    std::unique_ptr<PrimitiveMesh> quad, skyboxCube;
    std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> shaders;
    std::unordered_map<std::string, GLuint> textures;
    private:
};

}; 