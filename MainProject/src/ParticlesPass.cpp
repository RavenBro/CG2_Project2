#include "ParticlesPass.hpp"
#include "Renderer.hpp"

namespace renderer 
{
    void ParticlesPass::OnInit()
    {
        SetShader(GetRenderer()->GetShader("particles_s"));
        if(GetShader() == nullptr){
            throw(std::runtime_error("ParticlesPass::OnInit : shader not set"));
        }
        auto ls = GetScene()->lights.SVLights;
        for (int i=0;i<ls.GetCount();i++)
        {
            auto l = ls.Get(i);
            if (l.type == l.PARTICLES) 
            {
                p_systems.push_back(l.lps);
            }
            else if (l.type == l.POINT) 
            {
                ParticleSystem *l_part = new ParticleSystem();
                float rad = (l.type==l.POINT) ? 0.01 : l.radius;
                float3 color = l.color/(l.color.x+l.color.y+l.color.z);
                l_part->Init(1,l.position,0.01,color,float3(0,0,0),0,0,l_part->SQUARE,rad,0.2*rad,true,false,4);
                p_systems.push_back(l_part);
            }
            
        }
        auto pl = GetScene()->lights.simplePointLights;
        for (int i=0;i<pl.GetCount();i++)
        {
            auto l = pl.Get(i);
            ParticleSystem *l_part = new ParticleSystem();
            float rad = (l.color.x+l.color.y+l.color.z);
            float3 color = l.color/rad;
            l_part->Init(1,l.position,0.01,color,float3(0,0,0),0,0,l_part->SQUARE,0.005*sqrt(rad),0,true,false,4);
            p_systems.push_back(l_part);
        }
        auto sml = GetScene()->lights.SMPointLights;
        for (int i=0;i<sml.GetCount();i++)
        {
            auto l = sml.Get(i);
            ParticleSystem *l_part = new ParticleSystem();
            float rad = (l.color.x+l.color.y+l.color.z);
            float3 color = l.color/rad;
            l_part->Init(1,l.position,0.01,color,float3(0,0,0),0,0,l_part->SQUARE,0.005*sqrt(rad),0,true,false,4);
            p_systems.push_back(l_part);
        }
        auto vsml = GetScene()->lights.VSMLights;
        for (int i=0;i<vsml.GetCount();i++)
        {
            auto l = vsml.Get(i);
            ParticleSystem *l_part = new ParticleSystem();
            float rad = (l.color.x+l.color.y+l.color.z);
            float3 color = l.color/rad;
            l_part->Init(1,l.position,0.01,color,float3(0,0,0),0,0,l_part->SQUARE,0.005*sqrt(rad),0,true,false,4);
            p_systems.push_back(l_part);
        }
        auto cl = GetScene()->lights.CMLights;
        for (int i=0;i<cl.GetCount();i++)
        {
            auto l = cl.Get(i);
            ParticleSystem *l_part = new ParticleSystem();
            float rad = (l.color.x+l.color.y+l.color.z);
            float3 color = l.color/rad;
            l_part->Init(1,l.position,0.01,color,float3(0,0,0),0,0,l_part->SQUARE,0.005*sqrt(rad),0,true,false,4);
            p_systems.push_back(l_part);
        }
        auto rl = GetScene()->lights.RSMLights;
        for (int i=0;i<rl.GetCount();i++)
        {
            auto l = rl.Get(i);
            ParticleSystem *l_part = new ParticleSystem();
            float rad = (l.color.x+l.color.y+l.color.z);
            float3 color = l.color/rad;
            l_part->Init(1,l.position,0.01,color,float3(0,0,0),0,0,l_part->SQUARE,0.01*rad,0,true,false,4);
            p_systems.push_back(l_part);
        }
        ParticleSystem *test_ps = new ParticleSystem();
        test_ps->Init(400,float3(-0.253391, 0.81, -1.48683),0.04,float3(0.0,0,0),float3(0.0,0.0,0.0),
                      0.004,0.01,test_ps->SQUARE,0.001,0.0003,false,false, test_ps->ASH);
        p_systems.push_back(test_ps);
        test_ps = new ParticleSystem();
        test_ps->Init(400,float3(0.990104, 0.81, -0.374768),0.04,float3(0.0,0,0),float3(0.0,0.0,0.0),
                      0.004,0.01,test_ps->SQUARE,0.001,0.0003,false,false, test_ps->ASH);
        p_systems.push_back(test_ps);
        test_ps = new ParticleSystem();
        test_ps->Init(400,float3(0.615916, 0.81, 0.0206439),0.04,float3(0.0,0,0),float3(0.0,0.0,0.0),
                      0.004,0.01,test_ps->SQUARE,0.001,0.0003,false,false, test_ps->ASH);
        p_systems.push_back(test_ps);
        test_ps = new ParticleSystem();
        test_ps->Init(400,float3(-0.593332, 0.81, -1.06741),0.04,float3(0.0,0,0),float3(0.0,0.0,0.0),
                      0.004,0.01,test_ps->SQUARE,0.001,0.0003,false,false, test_ps->ASH);
        p_systems.push_back(test_ps);

        test_ps = new ParticleSystem();
        test_ps->Init(200,float3(3.3286,2.111,-2.098),0.1,float3(0.5,0.5,0.5),float3(0.5,0.3,0.1),0.003,0.008,
        test_ps->SQUARE,0.007,0.006,true,false,test_ps->JOINT);
        p_systems.push_back(test_ps);
        for (auto ps : p_systems)
        {
            GLuint  vert,col,ind,sz,vao;
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vert);
            glGenBuffers(1, &col);
            glGenBuffers(1, &ind);
            glGenBuffers(1, &sz);
            
            int n = ps->GetCount();
            float v_data[3*n];
            float c_data[3*n];
            float sz_data[n];
            uint32_t ind_data[n];
            for (int i=0;i<n;i++)
            {
                ind_data[i]=i;
                auto part = ps->GetParticlePos(i);
                v_data[3*i]=part.x;
                v_data[3*i+1]=part.y;
                v_data[3*i+2]=part.z;
                auto cl = ps->GetParticle(i).color;
                c_data[3*i]=cl.x;
                c_data[3*i+1]=cl.y;
                c_data[3*i+2]=cl.z;
                sz_data[i] = ps->GetParticleSize(i);
            }
        

            glBindVertexArray(vao);

            glBindBuffer(GL_ARRAY_BUFFER, vert);
            glBufferData(GL_ARRAY_BUFFER, n*3* sizeof(GL_FLOAT), v_data, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid *)0);
            glEnableVertexAttribArray(0);


            glBindBuffer(GL_ARRAY_BUFFER, sz);
            glBufferData(GL_ARRAY_BUFFER, n * sizeof(GL_FLOAT), sz_data, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
            glEnableVertexAttribArray(1);
            
            
            glBindBuffer(GL_ARRAY_BUFFER, col);
            glBufferData(GL_ARRAY_BUFFER, n*3* sizeof(GL_FLOAT), c_data, GL_STATIC_DRAW);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid *)0);
            glEnableVertexAttribArray(2);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ind);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, n * sizeof(uint32_t), ind_data, GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            GL_CHECK_ERRORS;
            vao_s.push_back(vao);
            part_counts.push_back(n);
            vert_bufs.push_back(vert);
        }
    }
    void ParticlesPass::Process()
    {
        //fprintf(stderr,"process");
        //glClearColor(0, 0, 0, 1);
        //glClear(GL_COLOR_BUFFER_BIT);
        //fprintf(stderr,"start \n");
        for (int i=0;i<vao_s.size();i++)
        {
            p_systems[i]->Recalculate();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vert_bufs[i]);
            GL_CHECK_ERRORS;
            void *ptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
            GL_CHECK_ERRORS;
            float *v_data = (float *)ptr;
            for (int j=0;j<p_systems[i]->GetCount();j++)
            {
                
                auto part = p_systems[i]->GetParticlePos(j);
                v_data[3*j]=part.x;
                v_data[3*j+1]=part.y;
                v_data[3*j+2]=part.z;
                //fprintf(stderr,"j=%d (%f %f %f)\n",j, part.x,part.y,part.z);
            }
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            
            glBindVertexArray(vao_s[i]); 
            BindShader();
            //glDepthFunc(GL_ALWAYS);
            GetShader()->SetUniform("VP",mul(GetScene()->sceneCamera.projection,GetScene()->sceneCamera.view));
            GetShader()->SetUniform("type",p_systems[i]->GetType());
            int t = p_systems[i]->IsLight() ? 1 :5;
            if (p_systems[i]->ReflectionMadness()) t+=2;
            GetShader()->SetUniform("is_light",t);
            GL_CHECK_ERRORS;
            //fprintf(stderr,"part ");
            glDrawElements(GL_POINTS, part_counts[i], GL_UNSIGNED_SHORT, nullptr);
            GL_CHECK_ERRORS;
            //glDepthFunc(GL_LEQUAL);
            UnbindShader();
            glBindVertexArray(0);
        }
    }
};