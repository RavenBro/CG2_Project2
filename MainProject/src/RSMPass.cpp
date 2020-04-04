#include "RSMPass.hpp"
#include "Renderer.hpp"

namespace renderer {

void RSMPass::OnInit(){
    SetShader("rsm_s");
    BindShader();
    uniforms.albedoTex = TryGetUniformLoc("albedoTex");
    uniforms.lightColor = TryGetUniformLoc("lightColor");
    uniforms.lightPos = TryGetUniformLoc("lightPos");
    uniforms.model = TryGetUniformLoc("model");
    uniforms.modelColor = TryGetUniformLoc("modelColor");
    uniforms.projection = TryGetUniformLoc("projection");
    uniforms.useTex = TryGetUniformLoc("useTex");
    uniforms.view = TryGetUniformLoc("view");
    UnbindShader();
}

void RSMPass::Process(){    
    BindShader();
    GLuint targetFB;
    glGenFramebuffers(1, &targetFB);
    glBindFramebuffer(GL_FRAMEBUFFER, targetFB);
    
    for(int i = 0; i < GetScene()->lights.RSMLights.GetCount(); i++){
        auto &light = GetScene()->lights.RSMLights.Get(i);
        glUniform3f(uniforms.lightPos, light.position.x, light.position.y, light.position.z);
        glUniform3f(uniforms.lightColor, light.color.x, light.color.y, light.color.z);
        glUniformMatrix4fv(uniforms.view, 1, GL_TRUE, light.view.L());
        glUniformMatrix4fv(uniforms.projection, 1, GL_TRUE, light.projection.L());
        GL_CHECK_ERRORS;
        light.normalTex = GenerateTexture2D(NormalTexFormat(), light.mapWidth, light.mapHeight, 0);
        light.depthTex = GenerateTexture2D(DepthTexFormat(), light.mapWidth, light.mapHeight, 0);
        light.fluxTex = GenerateTexture2D(HDRColorTexFormat(), light.mapWidth, light.mapHeight, 0);
        light.worldTex = GenerateTexture2D(WorldTexFormat(), light.mapWidth, light.mapHeight, 0);
        GL_CHECK_ERRORS;
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light.fluxTex, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, light.normalTex, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, light.worldTex, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light.depthTex, 0);
        glReadBuffer(GL_NONE);
        
        GLenum buff[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2}; 
        glDrawBuffers(3, buff);
        GL_CHECK_ERRORS;
        glViewport(0, 0, light.mapWidth, light.mapHeight);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
        GL_CHECK_ERRORS;
        float4x4 model; // = scale4x4(float3(0.005, 0.005, 0.005));
        model.identity();
        for(int nodeId : GetScene()->rootNodes){
            drawNode(nodeId, model);
        }
    }
    
    UnbindShader();
    glDeleteFramebuffers(1, &targetFB);
}

void RSMPass::drawNode(int nodeId, const float4x4 &model){
    const auto& node = GetScene()->nodes[nodeId];
    float4x4 newModel = mul(model, node.matrix);
    if(node.meshId >= 0){
        for(const auto& prim : GetScene()->meshes[node.meshId].primitives){
            const Material &material = (prim.materialId < 0)? 
                GetScene()->defaultMaterial : GetScene()->materials[prim.materialId];
            setMaterial(material);
            draw(prim, newModel);
        }
    }
    for(int childId : node.children){
        drawNode(childId, newModel);
    }
}
void RSMPass::setMaterial(const Material &material){
    glUniform1ui(uniforms.useTex, material.albedoTex != 0);
    if(material.albedoTex){
        glUniform1i(uniforms.albedoTex, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.albedoTex);
    }
    glUniform3f(uniforms.modelColor, material.color.x, 
        material.color.y, material.color.z);
    GL_CHECK_ERRORS;
}
void RSMPass::draw(const Mesh::Primitive &prim, const float4x4 &model){
    glUniformMatrix4fv(uniforms.model, 1, GL_TRUE, model.L());
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