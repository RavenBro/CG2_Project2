#include "Renderer.hpp"

#include "framework/HelperGL.h"

namespace renderer {

using namespace LiteMath;

static GLuint genSimpleCubemap();

void Renderer::Init(Scene *scene, int4 viewport){
    screenViewport = viewport;
    targetScene = scene;
    simpleCubemap = genSimpleCubemap();
    if(simpleCubemap == 0){
        throw(std::runtime_error("Renderer::Init - simpleCubemap gen error"));
    }
    quad.reset(CreateQuad());
    LoadShaders();
    skyboxCube.reset(CreateSkyBoxMesh());

    if(config.useShadows){
        shadowMaps.Init(this, scene, "SM");
        shadowMaps.Process();
        vsMaps.Init(this, scene, "VSM");
        vsMaps.Process();
        cmShadows.Init(this, scene, "CMShadows");
        cmShadows.Process();
        rsm.Init(this, scene, "RSM");
        rsm.Process();

        CSShadows.Init(this,scene,"CSShadows");
        CSShadows.Process();
        
    }
    integrateBRDF.Init(this, scene, "BRDFLuT");
    integrateBRDF.Process();
    
    envCubemapTransf.Init(this, scene, "GenCubemap");
    envCubemapTransf.Process();
    
    IBLcubemapGen.Init(this, scene, "IBLGen");
    integrateSpec.Init(this, scene, "IBLSpec");
    

    gbuffer.Init(this, scene, "GBuffer");
    
    svShadows.Init(this,scene,"SVShadows");
    penumbraShadows.Init(this,scene,"PenumbraShadows");
    gradient.Init(this,scene,"Gradient");
    CMRPass.Init(this, scene,"CMReflections");
    skybox.Init(this, scene, "Skybox");
    ambientOcclusion.Init(this, scene, "AO");
    pbr.Init(this, scene, "PBR");
    if(config.useIndirectLighting)
        indirLight.Init(this, scene, "IL");
    if(config.useReflections)
        reflects.Init(this, scene, "Ref");
    assembler.Init(this, scene, "Asm");
    part.Init(this, scene, "Particle");
    texDraw.Init(this, scene, "TextureDrawer");

    IBLcubemapGen.Process();
    integrateSpec.Process();
    CMRPass.Render();
    GL_CHECK_ERRORS;
}

ShaderProgram *Renderer::GetShader(const std::string &name) {
    auto iter = shaders.find(name);
    if(iter == shaders.end()) return nullptr;
    return iter->second.get();
} 

GLuint Renderer::GetTexture(const std::string &name){
    auto iter = textures.find(name);
    if(iter == textures.end()) return 0;
    return iter->second;
}

bool Renderer::AddTextureName(const std::string &name, GLuint tex){
    auto iter = textures.find(name);
    if(iter != textures.end()){
        return false;
    }
    textures[name] = tex;
    return true;
}

void Renderer::LoadShaders(){
    shaders.emplace("gbuffer_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/gbuffer/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/gbuffer/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;
    shaders.emplace("textureDraw_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/vsm_buff_draw/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/vsm_buff_draw/fragment.glsl"}
    }));
    shaders.emplace("pbr_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/pbr_lighting/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/pbr_lighting/fragment.glsl"}
    }));
    shaders.emplace("asm_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/assemble/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/assemble/fragment.glsl"}
    }));
    shaders.emplace("reflections_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/screen_reflection/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/screen_reflection/fragment.glsl"}
    }));
    shaders.emplace("blur_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/gaucean_blur/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/gaucean_blur/fragment.glsl"}
    }));
    shaders.emplace("hbao_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/HBAO/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/HBAO/fragment.glsl"}
    }));
    shaders.emplace("shadowmap_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/shadowmap/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/shadowmap/fragment.glsl"}
    }));
    shaders.emplace("vsm_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/vsm/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/vsm/fragment.glsl"}
    }));
    shaders.emplace("cmshadow_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/cmshadow/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/cmshadow/fragment.glsl"}
    }));
    shaders.emplace("skybox_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/SkyBox/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/SkyBox/fragment.glsl"}
    }));
    shaders.emplace("rsm_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/rsm/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/rsm/fragment.glsl"}
    }));
    shaders.emplace("indirectlight_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/indirectlight/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/indirectlight/fragment.glsl"}
    }));
    shaders.emplace("gencubemap_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/gencubemap/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/gencubemap/fragment.glsl"}
    }));
    shaders.emplace("integratediffuse_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/integratediffuse/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/integratediffuse/fragment.glsl"}
    }));
    shaders.emplace("integratebrdf_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/integratebrdf/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/integratebrdf/fragment.glsl"}
    }));
    shaders.emplace("integratespec_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/integratespec/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/integratespec/fragment.glsl"}
    }));
    shaders.emplace("reflectblur_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/reflectblur/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/reflectblur/fragment.glsl"}
    }));
    shaders.emplace("textureDrawScale_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/textureDrawScale/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/textureDrawScale/fragment.glsl"}
    }));
    shaders.emplace("stencil_test_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/stencil_test/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/stencil_test/fragment.glsl"}
    }));
    shaders.emplace("penumbra_drawer_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/penumbra_drawer/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/penumbra_drawer/fragment.glsl"}
    }));
    shaders.emplace("shadow_volumes_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/shadow_volumes/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/shadow_volumes/fragment.glsl"},
        {GL_GEOMETRY_SHADER, "shaders/shadow_volumes/geometry.glsl"}
    }));
    shaders.emplace("penumbra_wedges_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/penumbra_wedges/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/penumbra_wedges/fragment.glsl"},
        {GL_GEOMETRY_SHADER, "shaders/penumbra_wedges/geometry.glsl"}
    }));
    shaders.emplace("gradient_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/gradient/vertex.glsl"},
        {GL_GEOMETRY_SHADER, "shaders/gradient/geometry.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/gradient/fragment.glsl"}
    }));
    shaders.emplace("depth_draw_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/depth_draw/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/depth_draw/fragment.glsl"}
    }));
    shaders.emplace("particles_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/particles/vertex.glsl"},
        {GL_GEOMETRY_SHADER, "shaders/particles/geometry.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/particles/fragment.glsl"}
    }));
    shaders.emplace("CMReflections_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/CMReflections/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/CMReflections/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;
}

void Renderer::Draw(){

    CSShadows.Process();
    gbuffer.Process();
    if(config.useAO)
        ambientOcclusion.Process();
    skybox.Process();
    if(config.useIndirectLighting)
        indirLight.Process();

    svShadows.Process();
    penumbraShadows.Process();
    gradient.Process();

    pbr.Process();
    if(config.useReflections)
        reflects.Process();
    assembler.Process();
    texDraw.Process();
}

static GLuint genSimpleCubemap(){
    GLuint cubemap;
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    for(int i = 0; i < 6; i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return cubemap;
}

};