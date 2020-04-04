#include "SMPass.hpp"
#include "Renderer.hpp"
#include "Scenev2.hpp"
#include "GLTexture.hpp"
namespace renderer 
{

SMPass::~SMPass() {
}

void SMPass::OnInit() {
    SetShader("shadowmap_s");
    std::cout << "SMPass\n";
    auto& lights = GetScene()->lights.SMPointLights;
    for(int i = 0; i < lights.GetCount(); i++){
        ShadowMapPointLight& light = lights.Get(i);
        glGenFramebuffers(1, &light.framebuffer);
        std::cout << "GenTex\n";
        light.shadowMap = GenerateTexture2D(SMTexFormat(), light.mapWidth, light.mapHeight, 0);
        lights.Set(i, light);
        if(light.shadowMap == 0){
            throw(std::runtime_error("SMPass::OnInit shadow map gen. error"));
        }
        glBindFramebuffer(GL_FRAMEBUFFER, light.framebuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light.shadowMap, 0);
        glDrawBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    BindShader();
    uniforms.MVP = TryGetUniformLoc("MVP");
    UnbindShader();
}

void SMPass::Process(){
    glCullFace(GL_FRONT);
    BindShader();
    auto lights = GetScene()->lights.SMPointLights;
    for(int i = 0; i < lights.GetCount(); i++){
        float4x4 MV = lights.Get(i).viewPojection;
        assert(lights.Get(i).shadowMap != 0);
        glBindFramebuffer(GL_FRAMEBUFFER, lights.Get(i).framebuffer);
        glViewport(0, 0, lights.Get(i).mapWidth, lights.Get(i).mapHeight);
        glClearColor(1, 1, 1, 1);
        glClear(GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;
        for(auto nodeId : GetScene()->rootNodes){
            drawNode(nodeId, MV);
        }
    }   
    UnbindShader();
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SMPass::drawNode(int nodeId, const float4x4& mvp){
    const auto& node = GetScene()->nodes[nodeId];
    float4x4 newmvp = mul(mvp, node.matrix);
    if(node.meshId >= 0){
        for(const auto& prim : GetScene()->meshes[node.meshId].primitives){
            draw(prim, newmvp);
        }
    }
    for(int childId : node.children){
        drawNode(childId, newmvp);
    }
}

void SMPass::draw(const Mesh::Primitive& prim, const float4x4 &mvp){
    glUniformMatrix4fv(uniforms.MVP, 1, GL_TRUE, mvp.L());
    GL_CHECK_ERRORS;
    glBindVertexArray(prim.vao);
    GL_CHECK_ERRORS;
    if(prim.hasIndexies){
        assert(prim.vao != 0);
        assert(prim.count != 0); 
        assert(prim.count % 3 == 0);
        glDrawElements(prim.mode, prim.count, prim.type, prim.indexOffset);
        GL_CHECK_ERRORS;
    } else {
        glDrawArrays(prim.mode, 0, prim.count);
    }
    GL_CHECK_ERRORS;
}

};