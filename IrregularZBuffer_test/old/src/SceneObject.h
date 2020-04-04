#pragma once

#include <vector>
#include <LiteMath.h>
#include "framework/Objects3d.h"

#include "Material.h"

class ISceneObject
{
public:
    ISceneObject() 
    {
        material.metallic = 0.3;
        material.roughness = 0.7;
    }
    virtual ~ISceneObject(){}
    
    virtual void Draw(unsigned n) = 0;
    virtual unsigned GetMeshCount() = 0;
    virtual float4x4 GetTransform(unsigned n) = 0;
    virtual bool IsTextured() = 0;
    
    const PBRMaterial& GetMaterial() const { return material; }
    PBRMaterial& GetMaterial() { return material; }
    void SetMaterial(const PBRMaterial &mt) { material = mt; }

private:
    PBRMaterial material;
};

class SimpleSceneObject : public ISceneObject
{
public:   
    SimpleSceneObject() : ISceneObject() {}
    SimpleSceneObject(Mesh *m, const float4x4 trans) : mesh(m)
    {
        SetTransform(trans);
    }
    
    SimpleSceneObject(Mesh *m) : mesh(m)
    {
        modelM = mesh->GetTransform();
    }

    void SetTransform(const float4x4 transform) 
    { 
        modelM = mul(transform, mesh->GetTransform()); 
    }
    void SetMesh(Mesh *m) { mesh = m; }

    virtual unsigned GetMeshCount() override { return 1; }
    virtual float4x4 GetTransform(unsigned n) { return modelM; }
    
    virtual void Draw(unsigned n) override
    {
        mesh->Draw();
    }

    virtual bool IsTextured() override { return false; }

    virtual ~SimpleSceneObject() override {}
private:
    float4x4 modelM;
    Mesh *mesh = nullptr; 
    PBRMaterial material;
};

class ModelSceneObject : public ISceneObject
{
public:
    ModelSceneObject() : ISceneObject() {}
    ModelSceneObject(Model *m): model(m) 
    {
        if(model) 
        {
            for(auto& mesh : model->GetMeshes()) {
                modelM.push_back(mesh->GetTransform());
            }
        }
    }

    virtual void Draw(unsigned n) override
    {
        model->GetMeshes().at(n)->Draw();
    }
    
    virtual unsigned GetMeshCount() override
    {
        return model->GetMeshes().size();
    }
    
    virtual float4x4 GetTransform(unsigned n) override
    {
        return modelM.at(n);
    }
    
    void SetTransform(float4x4 transf, unsigned n)
    {
        modelM.at(n) = mul(transf, model->GetMeshes().at(n)->GetTransform());
    }

    void SetTransform(float4x4 transf, std::string name)
    {
        int i = -1;
        for(auto& mesh : model->GetMeshes())
        {
            ++i;
            if(mesh->GetName() == name) break;
        }
        if(i != -1) {
            modelM.at(i) = mul(transf, model->GetMeshes().at(i)->GetTransform());
            return;
        }
        throw std::logic_error("ModelSceneObject::SetTransfotm - name not found");
    }

    float4x4 GetTransform(std::string name)
    {
        int i = -1;
        for(auto& mesh : model->GetMeshes())
        {
            ++i;
            if(mesh->GetName() == name) break;
        }
        if(i != -1) {
            return modelM.at(i);
        }
        throw std::logic_error("ModelSceneObject::GetTransfotm - name not found");
    }

    virtual bool IsTextured() override { return true; }

    virtual ~ModelSceneObject() override {}

private:
    Model *model = nullptr;
    std::vector<float4x4> modelM;
};
