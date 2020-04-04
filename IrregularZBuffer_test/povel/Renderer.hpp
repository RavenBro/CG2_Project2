#pragma once

#include <unordered_map>
#include <memory>

#include "Scenev2.hpp"
#include "RenderPass.hpp" 
#include "framework/Primitives.h"

namespace renderer
{

class Renderer{
public:
    void Init(const Scene *scene, int4 viewport);
    void Draw();

    ShaderProgram *GetShader(const std::string &name);

    int4 GetScreenViewport() const { return screenViewport; }
    PrimitiveMesh *GetScreenQuad() const { return quad.get(); }

private:
    void LoadShaders();

    const Scene* targetScene = nullptr;
    
    int4 screenViewport;

    GBufferPass gbuffer;
    TextureDrawer texDraw;
    
    std::unique_ptr<PrimitiveMesh> quad;

    std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> shaders;
};

}; 