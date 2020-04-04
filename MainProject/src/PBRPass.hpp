#pragma once
#include "RenderPass.hpp"
#include "BlurPass.hpp"
#include "SkyboxPass.hpp"
#include "Light.hpp"

namespace renderer 
{

class PBRPass : public Filter {
public:
    virtual void OnInit() override;
    virtual void Process() override;


private:
    void UpdateLight(bool forced);
    void SetTexture(GLint loc, GLenum target, GLuint texture);
    void BindTextures();
    void SetDefaultTex(GLint loc, GLenum target);

    void UpdateComplexLight();
    struct {
        GLint simplePointLightBlock = -1, view = -1; 
        GLint diffuseTex = -1, normalTex = -1, vertexTex = -1, worldVTex,  materialTex = -1, skyboxTex = -1;
        GLint useAO = -1, aoTex = -1;
        GLint spLightsCount = -1;
        GLint useBloom = -1;
        GLuint partTex = -1; 
        GLint inverseCamM = -1; 
        GLint smLightsCount = -1;
        GLint vsmLightsCount = -1;
        GLint cmLightsCount = -1;
        
        struct {
            GLint position, color, normConeDir;
            float cosConeAngle;
            GLint viewProj;
            GLint shadowMap;
        } smLights[MAX_SM_LIGHT_COUNT], vsmLights[MAX_VSM_LIGHT_COUNT];

        struct {
            GLint color = -1, position = -1;
            GLint cubemap = -1;
            GLint zfar = -1;
        } cmLights[MAX_CM_LIGHT_COUNT];

        GLint useIBL;
        GLint irradianceMap;
        GLint specMap;
        GLint brdfLuT;
        GLint cameraPos;

        GLuint svTex = -1;
        GLuint ssvTex = -1;
        
    } uniforms;

    GLuint simplePointLightBuff = 0;
    GLuint diffuseTex = 0, normalTex = 0, vertexTex = 0, materialTex = 0, aoTex = 0, partTex = 0;
    BlurPass bloom;
    bool useBloom = false, useShadows = false;
    std::vector<std::pair<GLenum, GLuint>> textureLayout;
};

};