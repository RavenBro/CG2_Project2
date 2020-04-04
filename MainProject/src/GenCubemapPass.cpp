#include "GenCubemapPass.hpp"
#include "Renderer.hpp"
#include "LiteMath.h"

namespace renderer {

using namespace LiteMath;

const int CUBEMAP_SIZE = 512;

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

void GenCubemapPass::OnInit(){
    SetShader("gencubemap_s");
    BindShader();
    uniforms.equirectangularMap = TryGetUniformLoc("equirectangularMap");
    uniforms.view = TryGetUniformLoc("view");
    uniforms.projection = TryGetUniformLoc("projection");
    UnbindShader();
    GL_CHECK_ERRORS;
}

void GenCubemapPass::Process(){
    BindShader();
    GLuint FB = 0, depthRB = 0;
    glGenFramebuffers(1, &FB);
    glGenRenderbuffers(1, &depthRB);
    GL_CHECK_ERRORS;
    glBindFramebuffer(GL_FRAMEBUFFER, FB);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, CUBEMAP_SIZE, CUBEMAP_SIZE);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRB); 

    glGenTextures(1, &GetScene()->IBLcubemap.cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, GetScene()->IBLcubemap.cubemap);
    GL_CHECK_ERRORS;
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 
            CUBEMAP_SIZE, CUBEMAP_SIZE, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float4x4 projection = transpose4x4(
        projectionMatrixTransposed(90, 1, 0.1, 25.f));
    float4x4 views[6] {
        lookAtTransposed(float3(0.0f, 0.0f, 0.0f), float3( 1.0f,  0.0f,  0.0f), float3(0.0f, -1.0f,  0.0f)),
        lookAtTransposed(float3(0.0f, 0.0f, 0.0f), float3(-1.0f,  0.0f,  0.0f), float3(0.0f, -1.0f,  0.0f)),
        lookAtTransposed(float3(0.0f, 0.0f, 0.0f), float3( 0.0f,  1.0f,  0.0f), float3(0.0f,  0.0f,  1.0f)),
        lookAtTransposed(float3(0.0f, 0.0f, 0.0f), float3( 0.0f, -1.0f,  0.0f), float3(0.0f,  0.0f, -1.0f)),
        lookAtTransposed(float3(0.0f, 0.0f, 0.0f), float3( 0.0f,  0.0f,  1.0f), float3(0.0f, -1.0f,  0.0f)),
        lookAtTransposed(float3(0.0f, 0.0f, 0.0f), float3( 0.0f,  0.0f, -1.0f), float3(0.0f, -1.0f,  0.0f))
    };
    for(int i = 0; i < 6; i++){
        views[i] = transpose4x4(views[i]);
    }
    GL_CHECK_ERRORS;
    glUniform1i(uniforms.equirectangularMap, 0);
    glUniformMatrix4fv(uniforms.projection, 1, GL_TRUE, projection.L());
    GL_CHECK_ERRORS;
    glBindFramebuffer(GL_FRAMEBUFFER, FB);
    for(int side = 0; side < 6; side++){
        glUniformMatrix4fv(uniforms.view, 1, GL_TRUE, views[side].L());
        GL_CHECK_ERRORS;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, GetScene()->IBLcubemap.cubemap, 0);
        GL_CHECK_ERRORS;
        glReadBuffer(GL_NONE);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        std::cout << (int)(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) << "\n";
        GL_CHECK_ERRORS;
        glViewport(0, 0, CUBEMAP_SIZE, CUBEMAP_SIZE);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GetScene()->envMap);

        GetRenderer()->GetSkyboxCube()->Draw();
        GL_CHECK_ERRORS;
    }
    GL_CHECK_ERRORS;
    glDeleteRenderbuffers(1, &depthRB);
    glDeleteFramebuffers(1, &FB);
    UnbindShader(); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK_ERRORS;
    //glGenerateTextureMipmap(GetScene()->skybox);
}

};