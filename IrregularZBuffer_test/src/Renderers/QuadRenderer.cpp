#include "Renderers.h"

    void QuadRenderer::Render()
    {
        auto shader = GetShader();
        shader->StartUseShader();
        shader->SetUniform("vsm_tex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, targetTexture);
        //glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, targetTexture);

        for(auto obj : GetObjects()){
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
        shader->StopUseShader();
    }