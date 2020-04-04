#include "RenderPass.hpp"
#include "Renderer.hpp"

namespace renderer {

void RenderPass::SetShader(const std::string &sname){
    auto prog = GetRenderer()->GetShader(sname);
    if(prog == nullptr){
        std::string messg = GetName() + " : cannot find shader " + sname;
        throw(std::runtime_error(messg.c_str()));
    }
    SetShader(prog);
}

bool RenderPass::RegisterTexture(GLuint texture, const std::string &texname) {
    std::cout<<"Registered "+name + "/" + texname+"\n";
    return renderer->AddTextureName(name + "/" + texname, texture);
}

GLint RenderPass::TryGetUniformLoc(const std::string &name){
    auto res = glGetUniformLocation(shader->GetProgram(), name.c_str());
    if(res == -1){
        std::string messg = GetName() + " : cannot find uniform " + name;
        throw(std::runtime_error(messg.c_str()));
    }
    return res;
}

GLuint RenderPass::TryGetTexture(const std::string &name){
    GLuint tex = GetRenderer()->GetTexture(name);
    if(tex == 0){
        std::string messg = GetName() + " : cannot find shader " + name;
        throw(std::runtime_error(messg.c_str()));
    }
    return tex;
}

void TextureDrawer::OnInit() {
    SetShader(GetRenderer()->GetShader("textureDrawScale_s"));
    if(GetShader() == nullptr){
        throw(std::runtime_error("TextureDrawer::OnInit : shader not set"));
    }
    quadMesh = GetRenderer()->GetScreenQuad();
    targetTex = GetRenderer()->GetTexture("Asm/Result");
    if (false)
    {
        targetTex = 0;
        TL = GetRenderer()->GetTexture("Asm/Result");  
        TR = GetRenderer()->GetTexture("PBR/Bloom");
        BL = GetRenderer()->GetTexture("AO/Tex");
        BR = GetRenderer()->GetTexture("GBuffer/Albedo");
    }
    if (false)
    {
        targetTex = 0;
        TL = GetRenderer()->GetTexture("Asm/Result");  
        TR_type = 1;
        TR = GetScene()->lights.SMPointLights.Get(0).shadowMap;
        BL = GetRenderer()->GetTexture("GBuffer/Particles");
        BR = GetScene()->lights.VSMLights.Get(1).shadowMap;
    }
    if (false)
    {
        targetTex = 0;
        TL = GetRenderer()->GetTexture("Asm/Result");  
        TR = GetRenderer()->CSShadows.GetCascade(0);
        BL = GetRenderer()->CSShadows.GetCascade(1);
        BR = GetRenderer()->CSShadows.GetCascade(2);
    }
    if (false)
    {
        targetTex = 0;
        TL = GetRenderer()->GetTexture("Asm/Result");  
        TR = GetRenderer()->GetTexture("Gradient/Grad");
        BL = GetRenderer()->GetTexture("PenumbraShadows/Shade");
        BR = GetRenderer()->GetTexture("SVShadows/Shade");
    }
    fprintf(stderr,"Drawing Texture = %d\n",targetTex);
    if((targetTex == 0)&&(TL*TR*BL*BR == 0)){
        throw(std::runtime_error("TextureDrawer::OnInit : texture(s) not found"));
    }
    BindShader();
    auto loc = GetUniformLoc("vsm_tex");
    if(loc == -1){
        throw(std::runtime_error("TextureDrawer::Init : vsm_tex location not found"));
    }
    glUniform1i(loc, 0);
    UnbindShader();
}
GLuint TextureDrawer::GetTarget()
{
    return targetTex;
}
void TextureDrawer::SetTarget(GLuint target)
{
    targetTex = target;
}
void TextureDrawer::Set4Targets(GLuint TL,GLuint TR,GLuint BL,GLuint BR,
                     int TL_t,int TR_t, int BL_t, int BR_t)
{
    this->TL = TL;
    this->TR = TR;
    this->BL = BL;
    this->BR = BR;
    TL_type = TL_t;
    TR_type = TR_t;
    BL_type = BL_t;
    BR_type = BR_t;
}
void TextureDrawer::DefaultTypes()
{
    target_type = 0;
    TL_type = 0;
    TR_type = 0;
    BL_type = 0;
    BR_type = 0;
}
void TextureDrawer::SetState(int state)
{
    DefaultTypes();
    switch (state)
    {
        case 1:
            target_type = 1;
            targetTex = GetScene()->lights.SMPointLights.Get(0).shadowMap;
            break;
        case 2:
            targetTex = GetRenderer()->GetTexture("Asm/Result");
            break;
        case 3:
            targetTex = 0;
            TL = GetRenderer()->GetTexture("Asm/Result");  
            TR = GetRenderer()->GetTexture("PBR/Bloom");
            BL = GetRenderer()->GetTexture("AO/Tex");
            BR = GetRenderer()->GetTexture("GBuffer/Albedo");
            break;
        case 4:
            targetTex = 0;
            TL = GetRenderer()->GetTexture("Asm/Result");  
            TR_type = 1;
            TR = GetScene()->lights.SMPointLights.Get(0).shadowMap;
            BL = GetRenderer()->GetTexture("GBuffer/Particles");
            BR = GetScene()->lights.VSMLights.Get(1).shadowMap;
            break;
        case 5:
            targetTex = 0;
            TL = GetRenderer()->GetTexture("Asm/Result");  
            TR = GetRenderer()->GetTexture("Gradient/Grad");
            BL = GetRenderer()->GetTexture("PenumbraShadows/Shade");
            BR = GetRenderer()->GetTexture("SVShadows/Shade");
            break;
        case 6:
            targetTex = GetRenderer()->GetTexture("AO/Tex");
            break;
        case 7:
            targetTex = GetRenderer()->GetTexture("AO/Noise");
            break;
        case 8:
            break;
        case 9:
            break;
        default:
            targetTex = GetRenderer()->GetTexture("Asm/Result");
            break;
    }
}
void TextureDrawer::Process() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    auto v = GetRenderer()->GetScreenViewport();
    glViewport(v.x, v.y, v.z, v.w);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    if (targetTex)
    {
        BindShader();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, targetTex);
        GetShader()->SetUniform("v_shiftx",(float)0);
        GetShader()->SetUniform("v_shifty",(float)0);
        GetShader()->SetUniform("v_scalex",(float)1.0);
        GetShader()->SetUniform("v_scaley",(float)1.0);
        GetShader()->SetUniform("type",target_type);
        quadMesh->Draw();
        UnbindShader();
    }
    else
    {
        BindShader();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TL);
        GetShader()->SetUniform("v_shiftx",(float)0.5);
        GetShader()->SetUniform("v_shifty",(float)0.5);
        GetShader()->SetUniform("v_scalex",(float)0.5);
        GetShader()->SetUniform("v_scaley",(float)0.5);
        GetShader()->SetUniform("type",TL_type);
        quadMesh->Draw();
        UnbindShader();
        BindShader();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TR);
        GetShader()->SetUniform("v_shiftx",(float)0.5);
        GetShader()->SetUniform("v_shifty",(float)-0.5);
        GetShader()->SetUniform("v_scalex",(float)0.5);
        GetShader()->SetUniform("v_scaley",(float)0.5);
        GetShader()->SetUniform("type",TR_type);
        quadMesh->Draw();
        UnbindShader();

        BindShader();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, BL);
        GetShader()->SetUniform("v_shiftx",(float)-0.5);
        GetShader()->SetUniform("v_shifty",(float)0.5);
        GetShader()->SetUniform("v_scalex",(float)0.5);
        GetShader()->SetUniform("v_scaley",(float)0.5);
        GetShader()->SetUniform("type",BL_type);
        quadMesh->Draw();
        UnbindShader();

        BindShader();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, BR);
        GetShader()->SetUniform("v_shiftx",(float)-0.5);
        GetShader()->SetUniform("v_shifty",(float)-0.5);
        GetShader()->SetUniform("v_scalex",(float)0.5);
        GetShader()->SetUniform("v_scaley",(float)0.5);
        GetShader()->SetUniform("type",BR_type);
        quadMesh->Draw();
        UnbindShader();
    }
    
    
    GL_CHECK_ERRORS;
}

};