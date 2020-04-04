#include "CaptureScene.hpp"
#include "Renderer.hpp"
#include "Scenev2.hpp"
const int SIDE_SIZE = 600;

namespace renderer {

static float4x4 CMview(float3 pos, int side){
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
    return transpose4x4(lookAtTransposed(pos, pos + deye, up));
}

const float3 CAM_POS {26.4235, 1, 14.8966};

void CaptureScene::OnInit(){
    SetShader("textureDraw_s");
    BindShader();
    uniforms.vsm_tex = TryGetUniformLoc("vsm_tex");
    glUniform1i(uniforms.vsm_tex, 0);
    UnbindShader();

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    int4 v = GetRenderer()->GetScreenViewport();

    for (int i = 0; i < 6; i++)
    { 
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, SIDE_SIZE, SIDE_SIZE, 0, GL_RGBA,
        GL_FLOAT, 0);
    }
    GL_CHECK_ERRORS;
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    GL_CHECK_ERRORS;
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    if(!tex || !RegisterTexture(tex, "Cubemap")){
        throw(std::runtime_error("CaptureScene::OnInit : tex gen err"));
    }
}

void CaptureScene::Process(){
    GLuint copyFB;
    glGenFramebuffers(1, &copyFB);
    
    //auto conf = GetRenderer()->GetConf();
    //bool useIBL = conf.useIBL;
    //conf.useIBL = false;
    auto oldCamera = GetScene()->sceneCamera;
    auto &camera = GetScene()->sceneCamera;

    GLuint srcTex = TryGetTexture("PBR/Frame");

    camera.projection = transpose4x4(projectionMatrixTransposed(
        90, 1, 0.01, 10
    ));

    camera.fovRad = M_PI_2;
    camera.zFar = 100;
    camera.zNear = 0.3;
    GL_CHECK_ERRORS;
    camera.position = CAM_POS;
    
    int4 vp = GetRenderer()->GetScreenViewport(); 
    for(int side = 0; side < 6; side++){
        camera.view = CMview(CAM_POS, side);
        std::cout << "Capture side " << side << "\n";
        GL_CHECK_ERRORS;
        GetRenderer()->Draw();
        
        BindShader();
        glBindFramebuffer(GL_FRAMEBUFFER, copyFB);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, tex, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glViewport(0, 0, SIDE_SIZE, SIDE_SIZE);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcTex);
        GetRenderer()->GetScreenQuad()->Draw();

        UnbindShader();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);   
        GL_CHECK_ERRORS;
    }
    

    glDeleteFramebuffers(1, &copyFB);
    GetScene()->sceneCamera = oldCamera;
    GL_CHECK_ERRORS;
}

};