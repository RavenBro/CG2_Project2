#include "ParticleSystem.hpp"

void ParticleSystem::Init(int count, float3 center, float max_rad,float3 effective_color,float3 color_disp,
              float min_speed, float max_speed, int type, float base_size, float size_dispersion, bool is_light,
              bool ref_madness, int mov_type)
{
    std::uniform_real_distribution<float> Floats(0.0, 1.0); 
    for (int i=0;i<min(count,MAX_PARTICLE_COUNT);i++)
    {
        pcl part;
        this->center = center;
        this->max_rad = max_rad;
        this->type = type;
        this->min_speed = min_speed;
        this->max_speed = max_speed;
        this->is_light = is_light;
        this->reflection_madness = ref_madness;
        this->movement_type = mov_type;
        part.size= base_size+size_dispersion*(1-2*Floats(generator));
        if (part.size<0.0001) part.size = base_size;
        float phi = Floats(generator)*2*M_PI;
        float psi = Floats(generator)*2*M_PI;
        float r = max_rad*pow(Floats(generator),0.33333);
        part.pos=r*float3(cos(phi)*cos(psi),sin(phi)*cos(psi),sin(psi));
        part.color = effective_color - color_disp + 
            2*float3(Floats(generator)*color_disp.x,Floats(generator)*color_disp.y,Floats(generator)*color_disp.z);
       //part.color = Floats(generator)*color_disp;
        if (movement_type==SIMPLE)
        {
            part.dir = float3(0,0,0.01);
        }
        else if (movement_type==ASH)
        {
            part.dir = float3(0,min_speed+(max_speed-min_speed)*Floats(generator),0);
        }
        else if (movement_type == JOINT)
        {
            part.dir = float3(0,0,0.01);
        }
       particles.push_back(part);
    }
    
}
void ParticleSystem::Init(int count, float3 center, float max_rad,float3 effective_color, bool is_light, int mov_type)
{
    Init(count,center,max_rad,effective_color,float3(1,1,1),0.0,max_rad/20.0,SQUARE,0.05,0.01,is_light,false, mov_type);
}
void ParticleSystem::Recalculate()
{
    pos_calculated = false;   
    rad_calculated = false;
    if (movement_type==SIMPLE)
    {
        static std::uniform_real_distribution<float> Floats(min_speed, max_speed);
        for (int i=0;i<particles.size();i++)
        {
            int chance = rand()%20;
            if (chance==1) particles[i].dir = float3(Floats(generator),Floats(generator),Floats(generator));
            else if (chance==0) particles[i].dir = -float3(Floats(generator),Floats(generator),Floats(generator));
            particles[i].pos+=particles[i].dir; 
            if (length(particles[i].pos)>max_rad) 
            {
                particles[i].pos = normalize(particles[i].pos)*max_rad;
                if (chance%2==1) particles[i].dir = float3(Floats(generator),Floats(generator),Floats(generator));
                else if (chance%2==0) particles[i].dir = -float3(Floats(generator),Floats(generator),Floats(generator));
            }
        }
    }
    else if (movement_type == ASH)
    {
        static std::uniform_real_distribution<float> Floats(0.2*min_speed, 0.2*max_speed);
        for (int i=0;i<particles.size();i++)
        {
            int chance = rand()%20;
            if (chance==1) 
            {
                particles[i].dir.x = Floats(generator);
                particles[i].dir.z = Floats(generator);
            }
            else if (chance==0) 
            {
                particles[i].dir.x = -Floats(generator);
                particles[i].dir.z = -Floats(generator);
            }
            particles[i].pos+=particles[i].dir; 
            if (sqrt(particles[i].pos.x*particles[i].pos.x+particles[i].pos.z*particles[i].pos.z)>max_rad) 
            {
                float3 p = float3(particles[i].pos.x,0,particles[i].pos.z);
                p = normalize(p)*max_rad;
                particles[i].pos.x = p.x;
                particles[i].pos.z = p.z;
                if (chance%2==1) 
                {
                    particles[i].dir.x = Floats(generator);
                    particles[i].dir.z = Floats(generator);
                }
                else if (chance%2==1) 
                {
                    particles[i].dir.x = -Floats(generator);
                    particles[i].dir.z = -Floats(generator);
                }
            }
            if (abs(particles[i].pos.y)>5*max_rad) particles[i].pos.y = 0;
            //sif (i==11) fprintf(stderr,"wtf %f %f %f",particles[i].dir.x,particles[i].dir.y,particles[i].dir.z);
        }
    }
    else if (movement_type == JOINT)
    {
        static std::uniform_real_distribution<float> Floats(0.5*min_speed, 0.5*max_speed);
        static float3 joint_dir = float3(0,0,0);
        int j_chance = rand()%20;
        if (j_chance==0)
        {
            joint_dir = float3(Floats(generator),Floats(generator),Floats(generator));
        }
        else if (j_chance==1)
        {
            joint_dir = -float3(Floats(generator),Floats(generator),Floats(generator));
        }
        for (int i=0;i<particles.size();i++)
        {
            int chance = rand()%20;
            if (chance==1) particles[i].dir = float3(Floats(generator),Floats(generator),Floats(generator));
            else if (chance==0) particles[i].dir = -float3(Floats(generator),Floats(generator),Floats(generator));
            particles[i].pos+=joint_dir+particles[i].dir; 
            if (length(particles[i].pos)>3*max_rad) 
            {
                particles[i].pos = normalize(particles[i].pos)*3*max_rad;
                if (chance%2==1) particles[i].dir = float3(Floats(generator),Floats(generator),Floats(generator));
                else if (chance%2==0) particles[i].dir = -float3(Floats(generator),Floats(generator),Floats(generator));
            }
        }
    }
}
float ParticleSystem::GetEffectiveRadius()
{
    static float ef_rad = 0.0;
    if (!rad_calculated) 
    {
        float3 ep = GetEffectivePos();
        float rad = 0.0;
        for (auto part : particles)
        {
            rad+=length(part.pos-ep+center);
        }
        ef_rad = rad/particles.size();
        rad_calculated = true;
    }
    
    //fprintf(stderr,"rad = %f  ",rad/particles.size());
    return ef_rad;
}
float3 ParticleSystem::GetEffectivePos()
{
    static float3 ef_pos = float3(0,0,0);
    if (!pos_calculated)
    {
        float3 pos = float3(0.0,0.0,0.0);
        for (auto part : particles)
        {
            pos+=part.pos;
        }
        ef_pos = center+pos/particles.size();
        pos_calculated = true;
    }
    
    //fprintf(stderr,"pos = %f %f %f\n",pos.x,pos.y,pos.z);
    return ef_pos;
}