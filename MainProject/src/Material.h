#pragma once

#include <LiteMath.h>

using namespace LiteMath;

struct PBRMaterial
{
    float metallic;
    float roughness;
    float reflection = 0;
    float3 color {0, 0, 0}; 
};