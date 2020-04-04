#include "Renderers.h"

    void Renderer::AddObject(ISceneObject *sobj)
    {
        if(!sobj)
            throw(std::logic_error("Obj is null"));
        objects.push_back(sobj);
    }

    void Renderer::DelObject(ISceneObject *sobj)
    {
        if(sobj)
        {
            for(auto iter = objects.begin(); iter != objects.end(); iter++)
            {
                if((*iter) == sobj)
                {
                    objects.erase(iter);
                    return;
                }
            }
            throw std::logic_error("No obj");
        }
    }
    void Renderer::DrawObjects()
    {
        for(auto obj : GetObjects())
        {
            for(auto i = 0u; i < obj->GetMeshCount(); i++)
            {
                obj->Draw(i);
            }
        }
    }


