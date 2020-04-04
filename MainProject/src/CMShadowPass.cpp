#include "CMShadowPass.hpp"
#include "Renderer.hpp"
#include "BlurPass.hpp"

namespace renderer 
{

static float4x4 CMviewProj(const CMPointLight &light, int side);
static void GenVSMCubemap(CMPointLight &light);


void CMShadowPass::OnInit(){
    SetShader("cmshadow_s");
    BindShader();
    uniforms.lightPos = TryGetUniformLoc("lightPos");
    uniforms.model = TryGetUniformLoc("model");
    uniforms.viewProj = TryGetUniformLoc("viewProj");
    uniforms.zfar = TryGetUniformLoc("zfar");
    UnbindShader();

}
void CMShadowPass::Process(){
    BlurPass blur;
    blur.Init(GetRenderer(), GetScene(), "Blur");
    blur.SetSigma(3);
    GL_CHECK_ERRORS;
    BindShader();
    auto& lights = GetScene()->lights.CMLights;
    for(int i = 0; i < lights.GetCount(); i++){
        auto &light = lights.Get(i);
        GenVSMCubemap(light);
        
        GLuint drawSideFB, depthRB;
        glGenFramebuffers(1, &drawSideFB);
        glGenRenderbuffers(1, &depthRB);
        GLuint sideTex = GenerateTexture2D(VSMTexFormat(), light.sideSize, light.sideSize, 0);
        
        if(i != 0){
            blur.Clear();
        }
        blur.SetTexture(sideTex, VSMTexFormat(), light.sideSize, light.sideSize, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, depthRB);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, light.sideSize, light.sideSize);

        glBindFramebuffer(GL_FRAMEBUFFER, drawSideFB);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRB);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sideTex, 0);
        glReadBuffer(GL_NONE);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        GLuint copyFB;
        glGenFramebuffers(1, &copyFB);
        
        BindShader();
        glUniform1f(uniforms.zfar, light.zfar);
        glUniform3f(uniforms.lightPos, light.position.x, light.position.y, light.position.z);
        UnbindShader();

        for(int side = 0; side < 6; side++){
            BindShader();
            glBindFramebuffer(GL_FRAMEBUFFER, drawSideFB);
            glViewport(0, 0, light.sideSize, light.sideSize);

            float4x4 viewProj = CMviewProj(light, side);
            glUniformMatrix4fv(uniforms.viewProj, 1, GL_TRUE, viewProj.L());
            glClearColor(1, 1, 1, 1);
            glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
            GL_CHECK_ERRORS;
            float4x4 model;
            model.identity();
            for(auto nodeId : GetScene()->rootNodes){
                drawNode(nodeId, model);
            }
            UnbindShader();
            blur.Process();
            glBindFramebuffer(GL_FRAMEBUFFER, copyFB);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sideTex, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, light.cubemap, 0);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            glViewport(0, 0, light.sideSize, light.sideSize);
            glBlitFramebuffer(0, 0, light.sideSize, light.sideSize, 
                0, 0, light.sideSize, light.sideSize, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }
        glDeleteFramebuffers(1, &drawSideFB);
        glDeleteRenderbuffers(1, &depthRB);
        glDeleteFramebuffers(1, &copyFB);
        glDeleteTextures(1, &sideTex);
    }
    UnbindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK_ERRORS;
}

void CMShadowPass::drawNode(int nodeId, const float4x4& model){
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

void CMShadowPass::draw(const Mesh::Primitive& prim, const float4x4 &model){
    glUniformMatrix4fv(uniforms.model, 1, GL_TRUE, model.L());
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

void CMShadowPass::setFramebuffer(CMPointLight &light, int side){
    if(side == 0){
        if(!light.framebuffer){
            glGenFramebuffers(1, &light.framebuffer);
            glGenTextures(1, &light.cubemap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, light.cubemap);

            for (int i = 0; i < 6; i++){ 
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                    light.sideSize, light.sideSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            /*glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);*/
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, light.framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, light.cubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glViewport(0, 0, light.sideSize, light.sideSize);
    GL_CHECK_ERRORS;
    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
    case GL_FRAMEBUFFER_UNDEFINED:
        std::cout << "undef\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cout << "incom att\n";
        break;
    case GL_FRAMEBUFFER_COMPLETE:
        std::cout << "win\n";
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cout << "unsup\n";
        break;
    default:
        std::cout << "Other\n";
        break;
    } 
}

static float4x4 CMviewProj(const CMPointLight &light, int side){
    auto pos = light.position;
    float3 deye, up;
    switch (side)
    {
    case 0:
        deye = float3(1, 0, 0);
        up = float3(0, -1, 0);
        break;
    case 1:
        deye = float3(-1,0,0);
        up = float3(0,-1,0);
    break;
    case 2:
        deye = float3(0,1,0);
        up = float3(0,0,1);
    break;
    case 3:
        deye  = float3(0,-1,0);
        up = float3(0,0,-1);
    break;
    case 4:
        deye = float3(0,0,1);
        up = float3(0,-1,0);
    break;
    case 5:
        deye = float3(0,0,-1);
        up = float3(0,-1,0);
    break;
    default:
        assert(0);
        break;
    }
    float4x4 projection = transpose4x4(
            projectionMatrixTransposed(90, 1, light.znear, light.zfar));
    return mul(projection, transpose4x4(lookAtTransposed(pos, pos + deye, up)));
}

static void GenVSMCubemap(CMPointLight &light){
    glGenTextures(1, &light.cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, light.cubemap);

    for (int i = 0; i < 6; i++){ 
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RG32F, 
            light.sideSize, light.sideSize, 0, GL_RG, GL_FLOAT, 0);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    /*glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);*/
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

};