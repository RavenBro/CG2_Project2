#include "VSMPass.hpp"
#include "Renderer.hpp"

namespace renderer 
{

void VSMPass::OnInit(){
    SetShader("vsm_s");
    BindShader();
    uniforms.MVP = TryGetUniformLoc("MVP");
    UnbindShader();
    blur.Init(GetRenderer(), GetScene(), "Blur");

}

void VSMPass::Process(){
    for(int i = 0; i <  GetScene()->lights.VSMLights.GetCount(); i++){
        auto &light = GetScene()->lights.VSMLights.Get(i);
        SetFB(light);
        BindShader();
        glClear(GL_DEPTH_BUFFER_BIT);
        for(auto nodeId : GetScene()->rootNodes){
            drawNode(nodeId, light.viewPojection);
        }
        UnbindShader();
        blur.SetTexture(light.shadowMap, VSMTexFormat(), light.mapWidth, light.mapHeight, 0);
        blur.SetSigma(light.blurKernel);
        blur.Process();
        blur.DeleteTempTex(); 
    }
    
}

void VSMPass::drawNode(int nodeId, const float4x4& model){
    const auto& node = GetScene()->nodes[nodeId];
    float4x4 newmvp = mul(model, node.matrix);
    if(node.meshId >= 0){
        for(const auto& prim : GetScene()->meshes[node.meshId].primitives){
            draw(prim, newmvp);
        }
    }
    for(int childId : node.children){
        drawNode(childId, newmvp);
    }
}

void VSMPass::draw(const Mesh::Primitive& prim, const float4x4 &model){
    glUniformMatrix4fv(uniforms.MVP, 1, GL_TRUE, model.L());
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

void VSMPass::SetFB(VSMLight &light){
    if(!light.framebuffer){
        glGenFramebuffers(1, &light.framebuffer);
    }
    if(!light.shadowMap){
        light.shadowMap = GenerateTexture2D(VSMTexFormat(), light.mapWidth, light.mapHeight, 0);
        if(!light.shadowMap){
            throw(std::runtime_error("VSMPass : texture gen error"));
        }
    }
    if(!light.depthTex){
        light.depthTex = GenerateTexture2D(DepthTexFormat(), light.mapWidth, light.mapHeight, 0);
        if(!light.depthTex){
            throw(std::runtime_error("VSMPass : texture gen error"));
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, light.framebuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light.shadowMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light.depthTex, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        throw(std::runtime_error("VSMPass: framebuffer incomplete"));
    }
    glViewport(0, 0, light.mapWidth, light.mapHeight);
}

};