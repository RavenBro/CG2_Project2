#pragma once

#include <LiteMath.h>
#include <glad/glad.h>
#include <array>
#include "ParticleSystem.hpp"

namespace renderer 
{

const int MAX_LIGHT_COUNT = 16;
const int MAX_SM_LIGHT_COUNT = 4;
const int MAX_VSM_LIGHT_COUNT = 2;
const int MAX_CM_LIGHT_COUNT = 2;
const int MAX_RSM_LIGHT_COUNT = 1;
const int MAX_SV_LIGHT_COUNT = 4;

using namespace LiteMath;

struct SimplePointLight {
    alignas(16) float3 position = float3(0, 0, 0);
    alignas(16) float3 color = float3(0, 0, 0);
};

struct ShadowMapPointLight : SimplePointLight {
    ShadowMapPointLight() {}
    ShadowMapPointLight(float3 c, float3 eye, float3 center, float3 up, float near, float far, int width, int height, float fovyRad){
        mapWidth = width;
        mapHeight = height;
        float asp = ((float)width)/height;
        viewPojection = transpose4x4(projectionMatrixTransposed(fovyRad * 180/M_PI, asp, near, far));
        viewPojection = mul(viewPojection, transpose4x4(lookAtTransposed(eye, center, up)));
        color = c;
        position = eye;
        normConeDir = normalize(eye - center);
        float coneAngle = std::min(fovyRad, fovyRad * asp)/2;
        cosConeAngle = cos(coneAngle);
    }

    float4x4 viewPojection;
    int mapWidth, mapHeight;
    float cosConeAngle;
    float3 normConeDir;
    GLuint framebuffer = 0, shadowMap = 0;
};

struct VSMLight : ShadowMapPointLight {
    VSMLight() : ShadowMapPointLight() {}
    VSMLight(float3 c, float3 eye, float3 center, float3 up, float near, float far, int width, int height, float fovyRad, float blur) :
        ShadowMapPointLight(c, eye, center, up, near, far, width, height, fovyRad), blurKernel(blur) {}
    float blurKernel = 0.f;
    GLuint depthTex = 0;
};

struct RSMLight : SimplePointLight {
    float4x4 view, projection;
    int mapWidth, mapHeight;
    float cosConeAngle;
    float3 normConeDir;
    GLuint depthTex = 0, fluxTex = 0, normalTex = 0, worldTex = 0;
};

struct CMPointLight : SimplePointLight {
    CMPointLight() {}
    CMPointLight(float3 c, float3 p, int size, float near, float far) :
    sideSize(size), znear(near), zfar(far) { position = p; color = c; }
    int sideSize = 0;
    float znear = 0, zfar = 0;
    GLuint framebuffer = 0, cubemap = 0; 
};

struct SVPointLight : SimplePointLight {
    SVPointLight() {}
    SVPointLight(int light_type, bool is_unique,float3 col,float3 pos, float rad, float max_shadow_len)
    {
        type = light_type;
        unique = is_unique;
        color = col;
        position = pos;
        radius = rad;
        shadow_len = max_shadow_len;
    }
    float GetEffectiveRadius() 
    {
        if (type!=PARTICLES) return radius;
        else return lps->GetEffectiveRadius();
    }
    float3 GetEffectivePos()
    {
        if (type!=PARTICLES) return position;
        else return lps->GetEffectivePos();
    }
    enum {POINT,SPHERE,PARTICLES};
    int type = POINT;
    bool unique = false;//источник с unique = false должен использоваться только для расчета теней, но
    //не освещения, предполагается, что в этой же точке будет установлен источник c другим типом. 
    //float3 position,color;
    float radius = 0.0;
    float shadow_len = 10.0;
    ParticleSystem *lps = nullptr;
    
};

template <typename T, std::size_t N>
struct LightState {
public:
    const T& Get(int n) const {
        return lights.at(n);
    }
    T& Get(int n) {
        modified = true;
        return lights.at(n);
    }
    void Set(int n, const T& newLight){
        modified = true;
        lights.at(n) = newLight;
    }

    int Size() const { return lights.size(); }
    bool IsModified() const { return modified; }
    void Update() { modified = false; }

    unsigned ByteSize() const {
        return lights.size() * sizeof(T);
    } 

    void *Data() const {
        return (void*)lights.data();
    }

    void SetLightsCount(int c){
        count = std::min((int)N, c);
    }
    int GetCount() const { return count; }
private:
    std::array<T, N> lights;
    bool modified = true;
    int count;
};

struct LightSystem {
    LightState<SimplePointLight, MAX_LIGHT_COUNT> simplePointLights;
    LightState<ShadowMapPointLight, MAX_SM_LIGHT_COUNT> SMPointLights;
    LightState<VSMLight, MAX_VSM_LIGHT_COUNT> VSMLights;
    LightState<CMPointLight, MAX_CM_LIGHT_COUNT> CMLights;
    LightState<RSMLight, MAX_RSM_LIGHT_COUNT> RSMLights;
    LightState<SVPointLight, MAX_SV_LIGHT_COUNT> SVLights;
};

};