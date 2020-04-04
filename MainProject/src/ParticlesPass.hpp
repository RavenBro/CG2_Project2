#pragma once

#include "RenderPass.hpp"
#include "ParticleSystem.hpp"
namespace renderer 
{
class ParticlesPass : public RenderPass {
public:

    virtual void OnInit() override;
    virtual void Process() override;

private:
    std::vector<ParticleSystem*> p_systems;
    std::vector<GLuint> vao_s;
    std::vector<GLuint> vert_bufs;
    std::vector<GLuint> size_bufs;
    std::vector<uint> part_counts; 
};
}
