#include "SkyboxPass.hpp"
#include "Renderer.hpp"

namespace renderer 
{

void SkyboxPass::OnInit(){
    SetShader("skybox_s");
    
    screenViewport = GetRenderer()->GetScreenViewport();

    BindShader();
    uniforms.projection = TryGetUniformLoc("projection");
    uniforms.view = TryGetUniformLoc("view");
    uniforms.skybox = TryGetUniformLoc("skybox");
    glUniform1i(uniforms.skybox, 0);
    UnbindShader();
    GL_CHECK_ERRORS;
    //skybox = GetScene()->IBLcubemap.specTex;
    skybox = GetScene()->skybox;
    //skybox = TryGetTexture("CaptureScene/Cubemap");
    if(skybox == 0){
        throw(std::runtime_error("Skybox pass skybox not found"));
    }    
    resultTex = GenerateTexture2D(ColorTexFormat(), 
        screenViewport.z, screenViewport.w, 0);
    if(resultTex == 0){
        throw(std::runtime_error("SkyboxPass tex gen err"));
    }
    RegisterTexture(resultTex, "Tex");
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, resultTex, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK_ERRORS;
}

void SkyboxPass::Process(){
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    BindShader();
    glViewport(screenViewport.x, screenViewport.y, screenViewport.z, screenViewport.w);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
    glUniformMatrix4fv(uniforms.projection, 1, GL_TRUE, GetScene()->sceneCamera.projection.L());
    float4x4 viewcpy = GetScene()->sceneCamera.view.L();
    viewcpy.row[0].w=0;
    viewcpy.row[1].w=0;
    viewcpy.row[2].w=0;
    viewcpy.row[3].x=0;
    viewcpy.row[3].y=0;
    viewcpy.row[3].z=0;
    glUniformMatrix4fv(uniforms.view, 1, GL_TRUE, viewcpy.L());
    GetRenderer()->GetSkyboxCube()->Draw();
    UnbindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK_ERRORS;
}

};