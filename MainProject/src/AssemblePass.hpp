#pragma once
#include "RenderPass.hpp"

namespace renderer 
{

class AssemblePass : public Filter {
public:
    void OnInit() override;
    void Process() override;

private:
    struct {
        GLint frameTex = -1, reflectTex = -1, reflectMask, materialTex = -1, useReflections = -1;
        GLint useBloom = -1, bloomTex = -1, partTex = -1;
        GLint useIndirectLight, indirectLight;
        GLint IBLReflectTex, FTex;
    } uniforms;
    GLuint frameTex = 0, reflectTex = 0, reflectMask = 0, materialTex = 0, bloomTex = 0, ILTex = 0,
     IBLReflectTex = 0, FTex = 0, partTex = 0;
};

};