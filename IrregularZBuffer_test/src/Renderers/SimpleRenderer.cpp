#include "Renderers.h"

    void SimpleRenderer::Render()
    {
        auto shader = GetShader();
        shader->StartUseShader();
        shader->SetUniform("projection", GetProj());
        shader->SetUniform("view", GetView());
        for(auto obj : GetObjects())
        {   
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
    }