#include "RenderPass.hpp"
#include "Renderer.hpp"

#include <LiteMath.h>
#include "framework/HelperGL.h"
#include <iostream>

#include "GLTexture.hpp"

namespace renderer 
{
using namespace LiteMath;

static void Show(float4x4 matr) {
    for (int i = 0; i < 4; i++) {
        std::cout<<matr.row[i].x<<" ";
        std::cout<<matr.row[i].y<<" ";
        std::cout<<matr.row[i].z<<" ";
        std::cout<<matr.row[i].w<<" ";
        std::cout<<std::endl;
    }
}

GBufferPass::~GBufferPass() {
    GLuint textures[] {colorTex, normalTex, depthTex, vertexTex, materialTex};
    glDeleteTextures(5, textures);
    glDeleteFramebuffers(1, &framebuffer);
}   

void GBufferPass::OnInit(){
    targetScene = GetScene();
    windowH = GetRenderer()->GetScreenViewport().z;
    windowW = GetRenderer()->GetScreenViewport().w;
    GL_CHECK_ERRORS;
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;
    if(!RegisterTexture(colorTex, "Albedo")){
        throw(std::runtime_error("AddTextureName"));
    }

    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowW, windowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;
    if(!RegisterTexture(depthTex, "Depth")){
        throw(std::runtime_error("AddTextureName"));
    }
    
    glGenTextures(1, &normalTex);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);   
    GL_CHECK_ERRORS;
    if(!RegisterTexture(normalTex, "Normal")){
        throw(std::runtime_error("AddTextureName"));
    }

    glGenTextures(1, &vertexTex);
    glBindTexture(GL_TEXTURE_2D, vertexTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;
    if(!RegisterTexture(vertexTex, "Vertex")){
        throw(std::runtime_error("AddTextureName"));
    }

    glGenTextures(1, &materialTex);
    glBindTexture(GL_TEXTURE_2D, materialTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;
    if(!RegisterTexture(materialTex, "Material")){
        throw(std::runtime_error("AddTextureName"));
    }
    glGenTextures(1, &particlesTex);
    glBindTexture(GL_TEXTURE_2D, particlesTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;
    if(!RegisterTexture(particlesTex, "Particles")){
        throw(std::runtime_error("AddTextureName"));
    }
    worldTex = GenerateTexture2D(WorldTexFormat(), windowW, windowH, 0);
    if(!RegisterTexture(worldTex, "World")){
        throw(std::runtime_error("GBuffer::OnInit world vertex tex gen err"));
    }
    worldNormTex = GenerateTexture2D(NormalTexFormat(), windowW, windowH, 0);
    if(!RegisterTexture(worldNormTex, "WorldNorm")){
        throw(std::runtime_error("GBuffer::OnInit world vertex tex gen err"));
    }

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    GL_CHECK_ERRORS;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, vertexTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, materialTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, worldTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, worldNormTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, particlesTex, 0);
    GLuint dbuff[]= {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,
                     GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6};
    glDrawBuffers(7, dbuff);
    GL_CHECK_ERRORS;
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK_ERRORS;

    SetShader(GetRenderer()->GetShader("gbuffer_s"));
    if(GetShader() == nullptr){
        throw(std::runtime_error("GBuffer::Init : shader not set"));
    }    
    BindShader();
    uniforms.model = GetUniformLoc("model"); 
    assert(uniforms.model != -1);
    uniforms.view = GetUniformLoc("view");
    assert(uniforms.view != -1);
    uniforms.projection = GetUniformLoc("projection");
    assert(uniforms.projection != -1);
    uniforms.color = GetUniformLoc("color");
    assert(uniforms.color != -1);
    uniforms.metallic = GetUniformLoc("metallic");
    assert(uniforms.metallic != -1);
    uniforms.roughness = GetUniformLoc("roughness");
    assert(uniforms.roughness != -1);
    uniforms.reflection = GetUniformLoc("reflection");
    assert(uniforms.reflection != -1);
    uniforms.useDifTex = GetUniformLoc("use_diffuse_tex");
    assert(uniforms.useDifTex != -1);
    uniforms.difTex = GetUniformLoc("diffuse_tex");
    assert(uniforms.difTex != -1);

    uniforms.pbrTex = TryGetUniformLoc("pbrTex");
    uniforms.usePBRTex = TryGetUniformLoc("usePBRTex");
    UnbindShader();
}

void GBufferPass::Process(){
    GL_CHECK_ERRORS;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, windowW, windowH);
    GL_CHECK_ERRORS;
    BindShader();
    GL_CHECK_ERRORS;
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    glUniformMatrix4fv(uniforms.projection, 1, GL_TRUE, targetScene->sceneCamera.projection.L());
    glUniformMatrix4fv(uniforms.view, 1, GL_TRUE, targetScene->sceneCamera.view.L());
    GL_CHECK_ERRORS;
    float4x4 model; // = scale4x4(float3(0.005, 0.005, 0.005));
    model.identity();
    for(int nodeId : targetScene->rootNodes){
        drawNode(nodeId, model);
    }
    auto t = GetScene()->tank;
    GetShader()->SetUniform("usePBRTex",0);
    for (int i=0;i< t->GetMeshCount();i++)
    {
        auto tr = t->GetTransform(i);
        glUniformMatrix4fv(uniforms.model, 1, GL_TRUE, tr.L());
        GetShader()->SetUniform("metallic",(float)0.8);
        GetShader()->SetUniform("roughness",(float)0.2);
        GetShader()->SetUniform("reflection",(float)0.1);
        t->Draw(i);

    }
    UnbindShader();
    GetRenderer()->part.Process();
    GetRenderer()->CMRPass.Process();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBufferPass::drawNode(int nodeId, const float4x4 &model){
    const auto& node = targetScene->nodes[nodeId];
    float4x4 newModel = mul(model, node.matrix);
    if(node.meshId >= 0){
        for(const auto& prim : targetScene->meshes[node.meshId].primitives){
            const Material &material = (prim.materialId < 0)? 
                targetScene->defaultMaterial : targetScene->materials[prim.materialId];
            setMaterial(material);
            draw(prim, newModel);
        }
    }
    for(int childId : node.children){
        drawNode(childId, newModel);
    }
}

void GBufferPass::setMaterial(const Material &material){
    glUniform1f(uniforms.metallic, material.metallic);
    GL_CHECK_ERRORS;
    glUniform1f(uniforms.roughness, material.roughness);
    glUniform1f(uniforms.reflection, material.reflection);
    glUniform4f(uniforms.color, material.color.x, material.color.y, material.color.z, material.color.w);
    glUniform1ui(uniforms.useDifTex, material.albedoTex != 0);
    if(material.albedoTex){
        glUniform1i(uniforms.difTex, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.albedoTex);
    }
    glUniform1ui(uniforms.usePBRTex, material.metallicRoughnessTex != 0);
    if(material.metallicRoughnessTex != 0){
        glUniform1i(uniforms.pbrTex, 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material.metallicRoughnessTex);
    }
    GL_CHECK_ERRORS;
}
void GBufferPass::draw(const renderer::Mesh::Primitive &prim, const float4x4 &model){
    glUniformMatrix4fv(uniforms.model, 1, GL_TRUE, model.L());
    glBindVertexArray(prim.vao);
    //GL_CHECK_ERRORS;
    if(prim.hasIndexies){
        /*assert(prim.vao != 0);
        assert(prim.count != 0); 
        assert(prim.count % 3 == 0);*/
        glDrawElements(prim.mode, prim.count, prim.type, prim.indexOffset);
        //GL_CHECK_ERRORS;
    } else {
        glDrawArrays(prim.mode, 0, prim.count);
    }
    //GL_CHECK_ERRORS;
}

};