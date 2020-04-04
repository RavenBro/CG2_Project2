#include "IBLcubemapPass.hpp"
#include "Renderer.hpp"

namespace renderer{

const int IRRADIANCE_MAP_SIZE = 32;

void IBLcubemapPass::OnInit(){
    SetShader("integratediffuse_s");
    BindShader();
    uniforms.envMap = TryGetUniformLoc("envMap");
    uniforms.projection = TryGetUniformLoc("projection");
    uniforms.view = TryGetUniformLoc("view");
    UnbindShader();

    glGenTextures(1, &GetScene()->IBLcubemap.diffuseTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, GetScene()->IBLcubemap.diffuseTex);
    for(int i = 0; i < 6; i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
            GL_RGB16F, IRRADIANCE_MAP_SIZE, IRRADIANCE_MAP_SIZE, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void IBLcubemapPass::Process(){
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glBindTexture(GL_TEXTURE_CUBE_MAP, GetScene()->IBLcubemap.diffuseTex);
    BindShader();
    GLuint FB, depthRB;
    glGenFramebuffers(1, &FB);
    glGenRenderbuffers(1, &depthRB);
    glBindFramebuffer(GL_FRAMEBUFFER, FB);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, IRRADIANCE_MAP_SIZE, IRRADIANCE_MAP_SIZE);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRB);

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
    float3 upDirs[6] {
        float3(0, -1, 0),
        float3(0, -1, 0),
        float3(0, 0, 1),
        float3(0, 0, -1),
        float3(0, -1, 0),
        float3(0, -1, 0)
    };

    GL_CHECK_ERRORS;
    glUniform1i(uniforms.envMap, 0);
    glUniformMatrix4fv(uniforms.projection, 1, GL_TRUE, projection.L());

    glBindFramebuffer(GL_FRAMEBUFFER, FB);
    for(int side = 0; side < 6; side++){
        glUniformMatrix4fv(uniforms.view, 1, GL_TRUE, views[side].L());
        GL_CHECK_ERRORS;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, GetScene()->IBLcubemap.diffuseTex, 0);
        GL_CHECK_ERRORS;
        glReadBuffer(GL_NONE);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        std::cout << (int)(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) << "\n";
        GL_CHECK_ERRORS;
        glViewport(0, 0, IRRADIANCE_MAP_SIZE, IRRADIANCE_MAP_SIZE);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, GetScene()->IBLcubemap.cubemap);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, TryGetTexture("CaptureScene/Cubemap"));
        GetRenderer()->GetSkyboxCube()->Draw();
        GL_CHECK_ERRORS;
    }

    glDeleteFramebuffers(1, &FB);
    glDeleteRenderbuffers(1, &depthRB);
    UnbindShader();
}

};