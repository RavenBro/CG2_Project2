#include "PBRPass.hpp"
#include "Renderer.hpp"

//GL_TEXTURE30 - simple 2d tex, GL_TEXTURE31 simple cubemp tex

const int DEFAULT_TEX2D_SLOT = 30;
const int DEFAULT_TEXCUBE_SLOT = 31;

namespace renderer
{

void PBRPass::OnInit() {
    SetShader("pbr_s");
    SetViewport(GetRenderer()->GetScreenViewport());

    useBloom = GetRenderer()->GetConf().useBloom;

    std::vector<Filter::TexInfo> textureConf;
    Filter::TexInfo info;
    info.width = GetViewport().z;
    info.height = GetViewport().w;
    info.name = "Frame";
    info.fbTarget = GL_COLOR_ATTACHMENT0;
    info.fmt = HDRColorTexFormat();
    info.level = 0;
    textureConf.push_back(info);
    
    info.fbTarget = GL_COLOR_ATTACHMENT1;
    info.name = "Reflect";
    info.fmt = HDRColorTexFormat();
    info.level = 0;
    textureConf.push_back(info);

    info.fbTarget = GL_COLOR_ATTACHMENT2;
    info.name = "F";
    info.fmt = HDRColorTexFormat();
    info.level = 0;
    textureConf.push_back(info);
    
    if(useBloom){
        Filter::TexInfo info;
        info.width = GetViewport().z;
        info.height = GetViewport().w;
        info.name = "Bloom";
        info.fbTarget = GL_COLOR_ATTACHMENT3;
        info.fmt = HDRColorTexFormat();
        info.level = 0;
        textureConf.push_back(info);
    }
    for(auto i : textureConf){
        std::cout << i.name << "\n";
    }
    if(!SetTargetTextures(textureConf)){
        throw(std::runtime_error("PBRPass::Init - texture/framebuffer generation error"));
    }
    std::cout << "EndPbr\n";
    bool aoEnabled = GetRenderer()->GetConf().useAO;

    diffuseTex = TryGetTexture("GBuffer/Albedo");
    normalTex = TryGetTexture("GBuffer/WorldNorm");
    vertexTex = TryGetTexture("GBuffer/Vertex");
    materialTex = TryGetTexture("GBuffer/Material");

    if(aoEnabled){
        aoTex = TryGetTexture("AO/Tex");
        textureLayout.push_back({GL_TEXTURE_2D, aoTex});
    } else {
        aoTex = 0;
    }

    auto prog = BindShader();
    uniforms.simplePointLightBlock = glGetUniformBlockIndex(prog, "SimplePointLightsBlock");
    if(uniforms.simplePointLightBlock == -1){
        throw(std::runtime_error("PBR::Init uniform not found"));
    }
    uniforms.inverseCamM = TryGetUniformLoc("InverseCamM");
    uniforms.diffuseTex = TryGetUniformLoc("diffuseTex");
    uniforms.materialTex = TryGetUniformLoc("materialTex");
    uniforms.vertexTex = TryGetUniformLoc("vertexTex");
    //uniforms.worldVTex = TryGetUniformLoc("worldVTex");
    uniforms.normalTex = TryGetUniformLoc("normalTex");
    uniforms.skyboxTex = TryGetUniformLoc("skyboxTex");
    uniforms.spLightsCount = TryGetUniformLoc("spLightsCount");
    uniforms.aoTex = TryGetUniformLoc("aoTex");
    uniforms.useAO = TryGetUniformLoc("useAO");
    uniforms.useBloom = TryGetUniformLoc("useBloom");
    uniforms.smLightsCount = TryGetUniformLoc("smLightsCount");
    uniforms.vsmLightsCount = TryGetUniformLoc("vsmLightsCount");
    uniforms.cmLightsCount = TryGetUniformLoc("cmLightsCount");
    
    uniforms.useIBL = TryGetUniformLoc("useIBL");
    uniforms.irradianceMap = TryGetUniformLoc("irradianceMap");
    uniforms.cameraPos = TryGetUniformLoc("cameraPos");
    uniforms.specMap = TryGetUniformLoc("specMap");
    uniforms.brdfLuT = TryGetUniformLoc("brdfLuT");

    uniforms.svTex = TryGetUniformLoc("SVShadeTex");
    uniforms.ssvTex = TryGetUniformLoc("SVSoftShadeTex");
    uniforms.partTex = TryGetUniformLoc("particlesTex");

    useShadows = GetRenderer()->GetConf().useShadows;
    for(int i = 0; i < MAX_SM_LIGHT_COUNT; i++){
        std::string pref = "smLights[" + std::to_string(i) + "].";
        uniforms.smLights[i].color = TryGetUniformLoc((pref + "color").c_str());
        uniforms.smLights[i].position = TryGetUniformLoc((pref + "position").c_str());
        uniforms.smLights[i].shadowMap = TryGetUniformLoc((pref + "shadowMap").c_str());
        uniforms.smLights[i].viewProj = TryGetUniformLoc((pref + "viewProj").c_str());
        uniforms.smLights[i].cosConeAngle = TryGetUniformLoc((pref + "cosConeAngle").c_str());
        uniforms.smLights[i].normConeDir = TryGetUniformLoc((pref + "normConeDir").c_str());
    }
    for(int i = 0; i < MAX_VSM_LIGHT_COUNT; i++){
        std::string pref = "vsmLights[" + std::to_string(i) + "].";
        uniforms.vsmLights[i].color = TryGetUniformLoc((pref + "color").c_str());
        uniforms.vsmLights[i].position = TryGetUniformLoc((pref + "position").c_str());
        uniforms.vsmLights[i].shadowMap = TryGetUniformLoc((pref + "shadowMap").c_str());
        uniforms.vsmLights[i].viewProj = TryGetUniformLoc((pref + "viewProj").c_str());
        uniforms.vsmLights[i].cosConeAngle = TryGetUniformLoc((pref + "cosConeAngle").c_str());
        uniforms.vsmLights[i].normConeDir = TryGetUniformLoc((pref + "normConeDir").c_str());
    }
    for(int i = 0; i < MAX_CM_LIGHT_COUNT; i++){
        std::string pref = "cmLights[" + std::to_string(i) + "].";
        uniforms.cmLights[i].color = TryGetUniformLoc((pref + "color").c_str());
        uniforms.cmLights[i].position = TryGetUniformLoc((pref + "position").c_str());
        uniforms.cmLights[i].cubemap = TryGetUniformLoc((pref + "cubemap").c_str());
        SetDefaultTex(uniforms.cmLights[i].cubemap, GL_TEXTURE_CUBE_MAP);
        uniforms.cmLights[i].zfar = TryGetUniformLoc(pref + "zfar");
    }

    SetTexture(uniforms.diffuseTex, GL_TEXTURE_2D, diffuseTex);
    SetTexture(uniforms.normalTex, GL_TEXTURE_2D, normalTex);
    SetTexture(uniforms.vertexTex, GL_TEXTURE_2D, vertexTex);
    //SetTexture(uniforms.worldVTex, GL_TEXTURE_2D, TryGetTexture("GBuffer/World"));
    SetTexture(uniforms.materialTex, GL_TEXTURE_2D, materialTex);
    SetTexture(uniforms.skyboxTex, GL_TEXTURE_2D, TryGetTexture("Skybox/Tex"));
    SetTexture(uniforms.irradianceMap, GL_TEXTURE_CUBE_MAP, GetScene()->IBLcubemap.diffuseTex);
    SetTexture(uniforms.brdfLuT, GL_TEXTURE_2D, TryGetTexture("BRDFLuT/Tex"));
    SetTexture(uniforms.specMap, GL_TEXTURE_CUBE_MAP, GetScene()->IBLcubemap.specTex);
    if(aoEnabled) SetTexture(uniforms.aoTex, GL_TEXTURE_2D, aoTex);

    SetTexture(uniforms.svTex, GL_TEXTURE_2D, TryGetTexture("SVShadows/Shade"));
    SetTexture(uniforms.ssvTex, GL_TEXTURE_2D, TryGetTexture("Gradient/Grad"));
    SetTexture(uniforms.partTex, GL_TEXTURE_2D, TryGetTexture("GBuffer/Particles"));
    glUniform1i(uniforms.useAO, aoEnabled? 1:0);
    glUniform1ui(uniforms.useBloom, useBloom);
    GL_CHECK_ERRORS;
    UpdateLight(true);
    glUniformBlockBinding(prog, uniforms.simplePointLightBlock, 0);
    GL_CHECK_ERRORS;
    UnbindShader();

    if(useBloom){
        bloom.Init(GetRenderer(), GetScene(), "Bloom");
        bloom.SetTexture(TryGetTexture(GetName() + "/Bloom"), HDRColorTexFormat(), GetViewport().z, GetViewport().w, 0);
        bloom.SetSigma(5.f);
    }
}

void PBRPass::Process() {
    BindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, GetFramebuffer());
    glViewport(GetViewport().x, GetViewport().y, GetViewport().z, GetViewport().w);
    glClear(GL_COLOR_BUFFER_BIT);
    
    UpdateComplexLight();

    BindTextures();
    
    GL_CHECK_ERRORS;
    float *zb = GetRenderer()->CSShadows.GetCasZBorders(GetScene()->sceneCamera.projection);
    for (int i=0;i<3;i++)
    {
        GetShader()->SetUniform("csmLight.Cascade["+std::to_string(i)+"]",25+i);
        glActiveTexture(GL_TEXTURE25+i);
        glBindTexture(GL_TEXTURE_2D,GetRenderer()->CSShadows.GetCascade(i));
        GetShader()->SetUniform("csmLight.viewProj["+std::to_string(i)+"]",
                                GetRenderer()->CSShadows.GetCascadeMatr(i));
        GetShader()->SetUniform("csmLight.dist["+std::to_string(i)+"]",zb[i]);
        GetShader()->SetUniform("vsm_connected",0);
    }
    GL_CHECK_ERRORS;
    glUniform1ui(uniforms.useIBL, GetRenderer()->GetConf().useIBL);
    glUniform3f(uniforms.cameraPos, GetScene()->sceneCamera.position.x, 
        GetScene()->sceneCamera.position.y, GetScene()->sceneCamera.position.z);
    //glUniformMatrix4fv(uniforms.view, 1, GL_TRUE, GetScene()->sceneCamera.view.L());
    auto inv = inverse4x4(GetScene()->sceneCamera.view);
    glUniformMatrix4fv(uniforms.inverseCamM, 1, GL_TRUE, inv.L());
    GL_CHECK_ERRORS;
    RenderQuad();
    UnbindShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    if(useBloom){
        bloom.Process();
    }
    GL_CHECK_ERRORS;
}

void PBRPass::UpdateLight(bool forced) {
    auto &LS = GetScene()->lights;
    if(LS.simplePointLights.IsModified() || forced) {
        if(simplePointLightBuff == 0) {
            glGenBuffers(1, &simplePointLightBuff);
            glBindBuffer(GL_UNIFORM_BUFFER, simplePointLightBuff);
            glBufferData(GL_UNIFORM_BUFFER, LS.simplePointLights.ByteSize(), LS.simplePointLights.Data(), GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, simplePointLightBuff);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            GL_CHECK_ERRORS;
        } else {
            glBindBuffer(GL_UNIFORM_BUFFER, simplePointLightBuff);
            auto ptr = glMapBuffer(GL_UNIFORM_BUFFER, simplePointLightBuff);
            std::memcpy(ptr, LS.simplePointLights.Data(), LS.simplePointLights.ByteSize());
            glUnmapBuffer(GL_UNIFORM_BUFFER);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        glUniform1i(uniforms.spLightsCount, LS.simplePointLights.GetCount());
        LS.simplePointLights.Update();
    }
    if(GetRenderer()->GetConf().useShadows){
        if(LS.SMPointLights.IsModified() || forced) {
            glUniform1i(uniforms.smLightsCount, LS.SMPointLights.GetCount());
            for(int i = 0; i < LS.SMPointLights.GetCount(); i++){
                const auto &l = LS.SMPointLights.Get(i);
                glUniform3f(uniforms.smLights[i].position, l.position.x, l.position.y, l.position.z); 
                glUniform3f(uniforms.smLights[i].color, l.color.x, l.color.y, l.color.z);
                glUniformMatrix4fv(uniforms.smLights[i].viewProj, 1, GL_TRUE, l.viewPojection.L());
                //glUniform1i(uniforms.smLights[i].shadowMap, i + 5);
                SetTexture(uniforms.smLights[i].shadowMap, GL_TEXTURE_2D, l.shadowMap);
                glUniform1f(uniforms.smLights[i].cosConeAngle, l.cosConeAngle);
                glUniform3f(uniforms.smLights[i].normConeDir, l.normConeDir.x, l.normConeDir.y, l.normConeDir.z);
            }
            LS.SMPointLights.Update();
        }
        if(LS.VSMLights.IsModified() || forced) {
            glUniform1i(uniforms.vsmLightsCount, LS.VSMLights.GetCount());
            for(int i = 0; i < LS.VSMLights.GetCount(); i++){
                const auto &l = LS.VSMLights.Get(i);
                glUniform3f(uniforms.vsmLights[i].position, l.position.x, l.position.y, l.position.z); 
                glUniform3f(uniforms.vsmLights[i].color, l.color.x, l.color.y, l.color.z);
                glUniformMatrix4fv(uniforms.vsmLights[i].viewProj, 1, GL_TRUE, l.viewPojection.L());
                //glUniform1i(uniforms.vsmLights[i].shadowMap, i + 9);
                SetTexture(uniforms.vsmLights[i].shadowMap, GL_TEXTURE_2D, l.shadowMap);
                glUniform1f(uniforms.vsmLights[i].cosConeAngle, l.cosConeAngle);
                glUniform3f(uniforms.vsmLights[i].normConeDir, l.normConeDir.x, l.normConeDir.y, l.normConeDir.z);
            }
            LS.VSMLights.Update();
        }

        if(LS.CMLights.IsModified() || forced) {
            glUniform1i(uniforms.cmLightsCount, LS.CMLights.GetCount());
            for(int i = 0; i < LS.CMLights.GetCount(); i++){
                const auto &l = LS.CMLights.Get(i);
                glUniform3f(uniforms.cmLights[i].color, l.color.x, l.color.y, l.color.z);
                glUniform3f(uniforms.cmLights[i].position, l.position.x, l.position.y, l.position.z);
                //glUniform1i(uniforms.cmLights[i].cubemap, 11 + i);
                SetTexture(uniforms.cmLights[i].cubemap, GL_TEXTURE_CUBE_MAP, l.cubemap);
                glUniform1f(uniforms.cmLights[i].zfar, l.zfar);
            }
            LS.CMLights.Update();
        }
    }
    
}

void PBRPass::SetTexture(GLint loc, GLenum target, GLuint texture){
    GLint num = textureLayout.size();
    glUniform1i(loc, num);
    textureLayout.push_back({target, texture}); 
}

void PBRPass::BindTextures(){
    glActiveTexture(GL_TEXTURE0 + DEFAULT_TEXCUBE_SLOT);
    glBindTexture(GL_TEXTURE_CUBE_MAP, GetRenderer()->GetEmptyCubemapTex());

    int offset = 0;
    for(const auto &t : textureLayout){
        glActiveTexture(GL_TEXTURE0 + offset);
        glBindTexture(t.first, t.second);
        offset++;
    }
}

void PBRPass::SetDefaultTex(GLint loc, GLenum target){
    if(target == GL_TEXTURE_CUBE_MAP){
        glUniform1i(loc, DEFAULT_TEXCUBE_SLOT);
    } else {
        glUniform1i(loc, 0);
    }
}

void PBRPass::UpdateComplexLight(){
    int k=0,l=0;
    for (int i=0; i< GetScene()->lights.SVLights.GetCount();i++)
    {
        auto svl = GetScene()->lights.SVLights.Get(i);
        if (svl.type==svl.POINT)
        {
            GetShader()->SetUniform("svLights["+std::to_string(k)+"].position",svl.GetEffectivePos());
            GetShader()->SetUniform("svLights["+std::to_string(k)+"].color",svl.color);
            k++;
        }
        else
        {
            GetShader()->SetUniform("ssvLights["+std::to_string(l)+"].position",svl.GetEffectivePos());
            GetShader()->SetUniform("ssvLights["+std::to_string(l)+"].color",svl.color);
            l++;
        }
                
    }
    GetShader()->SetUniform("svLightsCount",k);
    GetShader()->SetUniform("ssvLightsCount",l);
}

};