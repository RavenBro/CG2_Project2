#include "IBLSpecPass.hpp"
#include "Renderer.hpp"

namespace renderer {

const int MAX_MIP_LEVEL = 5;
const int TEX_SIZE = 128;

void IBLSpecPass::OnInit(){
    SetShader("integratespec_s");
    BindShader();
    uniforms.envMap = TryGetUniformLoc("envMap");
    uniforms.roughness = TryGetUniformLoc("roughness");
    uniforms.projection = TryGetUniformLoc("projection");
    uniforms.view = TryGetUniformLoc("view");
    UnbindShader(); 

    glGenTextures(1, &GetScene()->IBLcubemap.specTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, GetScene()->IBLcubemap.specTex);
    
    int texSize = TEX_SIZE;
    for(int i = 0; i < 6; i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            0, GL_RGB16F, texSize, texSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void IBLSpecPass::Process(){
    glBindTexture(GL_TEXTURE_CUBE_MAP, GetScene()->IBLcubemap.specTex);
    int texSize = TEX_SIZE;
    float4x4 projection = transpose4x4(
        projectionMatrixTransposed(90, 1, 15, 25.f));
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

    GLuint FB, depthRB;
    glGenFramebuffers(1, &FB);
    glGenRenderbuffers(1, &depthRB);
    glBindFramebuffer(GL_FRAMEBUFFER, FB);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRB);

    BindShader();
    glUniform1i(uniforms.envMap, 0);
    glUniformMatrix4fv(uniforms.projection, 1, GL_TRUE, projection.L());
    
    texSize = TEX_SIZE;
    for(int mip = 0; mip < MAX_MIP_LEVEL; mip++){
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texSize, texSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRB);

        float r = ((float)mip)/(MAX_MIP_LEVEL - 1);
        glUniform1f(uniforms.roughness, r);
        for(int side = 0; side < 6; side++){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, GetScene()->IBLcubemap.cubemap);
            //glBindTexture(GL_TEXTURE_CUBE_MAP, TryGetTexture("CaptureScene/Cubemap"));
            glUniformMatrix4fv(uniforms.view, 1, GL_TRUE, views[side].L());
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, GetScene()->IBLcubemap.specTex, mip);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glViewport(0, 0, texSize, texSize);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            GetRenderer()->GetSkyboxCube()->Draw();
        }
        texSize /= 2;
    }

    UnbindShader();

    glDeleteFramebuffers(1, &FB);
    glDeleteRenderbuffers(1, &depthRB);
}

};