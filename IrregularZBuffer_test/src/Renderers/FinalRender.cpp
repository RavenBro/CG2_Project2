#include "Renderers.h"

void FinalRenderer::SetTextures(GLuint frame, GLuint reflect, GLuint material,
                     GLuint bloom, GLuint SSAO, GLuint shade, GLuint color, GLuint refract)
    {
        frameTex = frame; 
        reflectTex = reflect;
        materialTex = material;
        bloomTex = bloom;
        SSAOTex = SSAO;
        shadeTex = shade;
        colorTex = color;
        refractTex = refract;
    }

void FinalRenderer::Render(OutputController *oc)
    {
        auto shader = GetShader();
        shader->StartUseShader();
        int f[7];
        oc->GetDrawArray(f);
        shader->SetUniform("df",f[0]);
        shader->SetUniform("dr",f[1]);
        shader->SetUniform("dm",f[2]);
        shader->SetUniform("dsh",f[3]);
        shader->SetUniform("db",f[4]);
        shader->SetUniform("dss",f[5]);
        shader->SetUniform("dd",f[6]);
        shader->SetUniform("frameTex", 0);
        shader->SetUniform("reflectTex", 1);
        shader->SetUniform("materialTex", 2);
        shader->SetUniform("diffuseTex",3);
        shader->SetUniform("refractTex",4);
        shader->SetUniform("bloomTex", 5);
        shader->SetUniform("SSAOTex", 6);
        shader->SetUniform("shadeTex", 7);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frameTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, reflectTex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, materialTex);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, refractTex);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, bloomTex);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, SSAOTex);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, shadeTex);
        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }

        shader->StopUseShader();
    }