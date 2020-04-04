#pragma once

#include <LiteMath.h>
#include <glad/glad.h>
#include <array>
#include <random>
using namespace LiteMath;
using namespace std;
class ParticleSystem
{
public:
    typedef struct ParticleSystem_s
    {
        float3 pos;
        float size;
        float3 color;
        float3 dir;
    } pcl;
    void Init(int count, float3 center, float max_rad,float3 effective_color, bool is_light, int mov_type);
    void Init(int count, float3 center, float max_rad,float3 effective_color,float3 color_disp,
              float min_speed, float max_speed, int type, float base_size, float size_dispersion,bool is_light,
              bool ref_madness, int mov_type);
    void Recalculate();
    float3 GetEffectivePos();
    float GetEffectiveRadius();
    enum {SQUARE, CUBE};
    enum {MAX_PARTICLE_COUNT = 20000};
    enum {SIMPLE,ASH,JOINT};
    int GetType() {return type;}
    int GetCount() {return particles.size();}
    pcl GetParticle(int n) {return particles[n];}
    float3 GetParticlePos(int n) {return particles[n].pos+center;}
    float GetParticleSize(int n) {return particles[n].size;}
    bool IsLight() {return is_light; }
    bool ReflectionMadness() { return reflection_madness; }
private:
    int type;
    int movement_type;
    std::vector<pcl> particles;
    float3 center;
    float max_rad;
    float min_speed;
    float max_speed; 
    std::default_random_engine generator;
    bool is_light;
    bool reflection_madness;

    bool pos_calculated = false;
    bool rad_calculated = false;
};
