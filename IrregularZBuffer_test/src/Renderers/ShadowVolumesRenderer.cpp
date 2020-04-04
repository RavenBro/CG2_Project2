#include "Renderers.h"

#include <glad/glad.h>
#include <LiteMath.h>
#include <vector>
#include <string>
using std::string;
using std::vector;
bool calculateAdjacencyData(const vector<float> &positions,
                const vector<float> &normals,
                const vector<uint32_t> &indices )
{
    typedef struct edge_s
    {
        uint32_t a;
        uint32_t b;
        uint32_t c;
    } edge;
    int sz = indices.size();
    edge edges[sz];
    if (sz%3) throw(std::logic_error("Indices vector is broken. It must contain 3*n elements"));
    for (int i=0;i<sz;i+=3)
    {
        edges[i].a = indices[i];
        edges[i].b = indices[i+1];
        edges[i].c = indices[i+2];

        edges[i+1].a = indices[i+1];
        edges[i+1].b = indices[i+2];
        edges[i+1].c = indices[i];

        edges[i+2].a = indices[i+2];
        edges[i+2].b = indices[i];
        edges[i+2].c = indices[i+1];
        
        int fi = 3*indices[i];
        if (normals.size()==0) continue;
        float3 norm = float3(normals[fi],normals[fi+1],normals[fi+2]);//first vertex normal;
        
        //triangle vertexes
        float3 a = float3(positions[fi],positions[fi+1],positions[fi+2]);
        fi = 3*indices[i+1];
        float3 b = float3(positions[fi],positions[fi+1],positions[fi+2]);
        fi = 3*indices[i+2];
        float3 c = float3(positions[fi],positions[fi+1],positions[fi+2]);
        
        //real normal mush be in the same half-plane with normal taken from normals vector
        float3 rn = cross(b-a,c-a);
        if (dot(rn,norm)<0)
        {
            for (int k=0;k<3;k++)
            {
                auto tmp = edges[i+k].a;
                edges[i+k].a = edges[i+k].b;
                edges[i+k].b = tmp;
            }
            auto te = edges[i+1];
            edges[i+1] = edges[i+2];
            edges[i+2] = te; 
            
        }
        
    }
    for (int i=0;i<sz;i++)
    {
        fprintf(stderr,"%d %d %d\n",edges[i].a,edges[i].b,edges[i].c);
    }
    vector<uint32_t> adj_ind;
    for (int i=0;i<sz;i++)
    {
        for (int j=0;j<=sz;j++)
        {
            if (j==sz)
            {
                fprintf(stderr,"\n%d %d %d\n",edges[i].a,edges[i].b,edges[i].c);
                throw(std::logic_error("Given model is not a closed body"));
            }
            if ((edges[j].a==edges[i].b)&&(edges[j].b==edges[i].a))
            {
                adj_ind.push_back(edges[i].a);
                adj_ind.push_back(edges[j].c);
                break;
            }
        }
    }
    for (uint32_t ai : adj_ind)
    {
        printf("%d,",(int)ai);
    }
    return(false);
}
void ShadowVolumesRenderer::AddOccluder(ISceneObject *sobj)
{
    if(!sobj)
            throw(std::logic_error("Obj is null"));
        occluders.push_back(sobj);
}
void ShadowVolumesRenderer::SetShadeTex(GLuint shade)
{
    //shadeTex = shade;
}
void ShadowVolumesRenderer::Init(int w,int h,ISceneObject *quad)
{

    texWidth = w;
    texHeight = h;
    Quad = quad;

    glGenTextures(1, &shadeTex);
    glBindTexture(GL_TEXTURE_2D, shadeTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, texWidth, texHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
    GL_CHECK_ERRORS;

    glGenTextures(1, &depthStencilTex);
    glBindTexture(GL_TEXTURE_2D, depthStencilTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, texWidth, texHeight, 
                 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;   

    glGenFramebuffers(1, &FrameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadeTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);
    GLuint dbuff[]= {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, dbuff);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
ShadowVolumesRenderer::~ShadowVolumesRenderer()
{
    GLuint textures[] { depthStencilTex}; 
    glDeleteTextures(1, textures);
    glDeleteFramebuffers(1, &FrameBuf);
}
void ShadowVolumesRenderer::SetShaders(ShaderProgram *first, ShaderProgram *second, ShaderProgram *third,
                                       ShaderProgram *fourth, ShaderProgram *fifth)
{
    First = first;
    Second = second;
    Third = third;
    Fourth = fourth;
    Fifth = fifth;
}
void ShadowVolumesRenderer::Render(float4x4 view, float4x4 projection, float3 campos)
{
    static float3 lightpos = float3(1.55,8,-1.0);
    //lightpos.y+=0.01;
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glViewport(0, 0, texWidth, texHeight);
    glEnable(GL_STENCIL_TEST);  
    glEnable(GL_DEPTH_TEST);
    glStencilMask(0xFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_ZERO,GL_ZERO,GL_ZERO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadeTex);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    GL_CHECK_ERRORS;

        
        auto shader = First;
        shader->StartUseShader();
        shader->SetUniform("projection", projection);
        shader->SetUniform("view", view);
        for(auto obj : GetObjects())
        {   
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
        glClearColor(0.f, 0.0f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
        glDepthMask(GL_FALSE);
        //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        //glDepthMask(GL_TRUE);
        //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilFunc( GL_ALWAYS,0,0xFF);
        glStencilOp(GL_KEEP,GL_INCR,GL_KEEP);         

        shader = Second;
        shader->StartUseShader();
        shader->SetUniform("projection", projection);
        shader->SetUniform("view", view);
        shader->SetUniform("campos", campos);
        shader->SetUniform("front", 0);
        shader->SetUniform("light",lightpos);
        shader->SetUniform("sc",(float)10.0);
        for(auto obj : occluders)
        {   
            shader->SetUniform("use_diffuse_tex", (int)obj->IsTextured());
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                ((SimpleSceneObject *)obj)->DrawAdjacency(0);
            }
        }


        shader->SetUniform("front", 1);
        glStencilOp(GL_KEEP,GL_DECR,GL_KEEP);           

        for(auto obj : occluders)
        {   
            shader->SetUniform("use_diffuse_tex", (int)obj->IsTextured());
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                ((SimpleSceneObject *)obj)->DrawAdjacency(0);
                
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
    if (true)
    {  
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_EQUAL);
        glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
        glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
        
        

        shader = Fourth;
        shader->StartUseShader();
        shader->SetUniform("projection", projection);
        shader->SetUniform("view", view);
        shader->SetUniform("light",lightpos);
        shader->SetUniform("sc",(float)15.0);
        shader->SetUniform("dist",(float)6.0);
        for(auto obj : GetObjects())
        {   
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
        glDepthFunc(GL_LEQUAL);
    }
    
    glClear(GL_STENCIL_BUFFER_BIT);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
        glDepthMask(GL_FALSE);
        //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        //glDepthMask(GL_TRUE);
        //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilFunc( GL_ALWAYS,0,0xFF);
        glStencilOp(GL_KEEP,GL_INCR,GL_KEEP);         

        shader = Third;
        shader->StartUseShader();
        shader->SetUniform("projection", projection);
        shader->SetUniform("view", view);
        shader->SetUniform("campos", campos);
        shader->SetUniform("front", 0);
        shader->SetUniform("light",lightpos);
        shader->SetUniform("light_radius",(float)0.60);
        shader->SetUniform("sc",(float)15.0);
        for(auto obj : occluders)
        {   
            shader->SetUniform("use_diffuse_tex", (int)obj->IsTextured());
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                for (int j=1;j<=3;j++)
                {
                    shader->SetUniform("side", j);
                    ((SimpleSceneObject *)obj)->DrawAdjacency(0);
                }
            }
        }


        shader->SetUniform("front", 1);
        glStencilOp(GL_KEEP,GL_DECR,GL_KEEP);           

        for(auto obj : occluders)
        {   
            shader->SetUniform("use_diffuse_tex", (int)obj->IsTextured());
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                for (int j=1;j<=3;j++)
                {
                    shader->SetUniform("side", j);
                    ((SimpleSceneObject *)obj)->DrawAdjacency(0);
                }
                
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
      
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
    if (true)
    {
        glDepthFunc(GL_EQUAL);
        glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
        glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
        
        

        shader = Fifth;
        shader->StartUseShader();
        shader->SetUniform("projection", projection);
        shader->SetUniform("view", view);
        shader->SetUniform("light",lightpos);
        shader->SetUniform("sc",(float)15.0);
        shader->SetUniform("dist",(float)6.0);
        for(auto obj : GetObjects())
        {   
            for(unsigned i = 0; i < obj->GetMeshCount(); i++)
            {
                shader->SetUniform("model", obj->GetTransform(i));
                obj->Draw(i);
            }
        }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
        glDepthFunc(GL_LEQUAL);
    }
    /*glDisable(GL_DEPTH_TEST);;
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
    glClear(GL_COLOR_BUFFER_BIT);
    shader = Second;
    shader->StartUseShader();
    Quad->Draw(0);
    shader->StopUseShader();
    glEnable(GL_DEPTH_TEST);*/

    glDisable(GL_STENCIL_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}