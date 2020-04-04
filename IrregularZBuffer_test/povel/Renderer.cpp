#include "Renderer.hpp"

#include "framework/HelperGL.h"

namespace renderer {

using namespace LiteMath;

void Renderer::Init(const Scene *scene, int4 viewport){
    screenViewport = viewport;
    targetScene = scene;

    quad.reset(CreateQuad());
    LoadShaders();
    gbuffer.SetShader(shaders.at("gbuffer_s").get());
    gbuffer.Init(800, 600, targetScene);
    
    texDraw.SetShader(shaders.at("textureDraw_s").get());
    texDraw.Init(quad.get(), gbuffer.GetAlbedoTex());
}

ShaderProgram *Renderer::GetShader(const std::string &name) {
    auto iter = shaders.find(name);
    if(iter == shaders.end()) return nullptr;
    return iter->second.get();
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
    GL_CHECK_ERRORS;
}

void Renderer::Draw(){
    gbuffer.Process();
    texDraw.Process();
}

};