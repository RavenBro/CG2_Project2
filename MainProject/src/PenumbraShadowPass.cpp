#include "Renderer.hpp"
#include "PenumbraShadowPass.hpp"
#include <glad/glad.h>
#include <LiteMath.h>
#include <vector>
#include <string>
#include <cstdio>
using std::string;
using std::vector;


Mesh *calculateAdjacencyData2(const char *path)
{
    FILE *src = fopen(path,"r");
    float tmp;
    
    vector<float> positions2;
    vector<float> normals2;
    vector<float> positions;
    vector<float> normals;
    vector<uint32_t> indices;
    while (fscanf(src,"%f ",&tmp))
    {   
        positions2.push_back(tmp);
    }
    if (fgetc(src)==',');// fprintf(stderr,"pos readed %d\n",positions2.size());
    while (fscanf(src,"%f ",&tmp))
    {
        normals2.push_back(tmp);
    }
    if (fgetc(src)==',');// fprintf(stderr,"norm readed %d\n",normals2.size());
    while (fscanf(src,"%f ",&tmp))
    {
        //текстурные координаты нам не нужны
    }
    if (fgetc(src)==',');// fprintf(stderr,"texc readed \n");
    int it;
    while (fscanf(src,"%d",&it))
    {
        indices.push_back((uint32_t)it);
    }
     //fprintf(stderr,"ind readed %d\n",indices.size());
     uint32_t *new_ind = (uint32_t *)calloc(positions2.size()/3,sizeof(uint32_t));
    #define eps 0.000001
    for (int i=0;i<positions2.size();i+=3)
    {
        int clone = -1;
        for (int j=0;j<positions.size();j+=3)
        {
            if ((abs(positions2[i]-positions[j])<eps)&&
                (abs(positions2[i+1]-positions[j+1])<eps)&&
                (abs(positions2[i+2]-positions[j+2])<eps))
            {
                //fprintf(stderr,"found clone\n");
                clone = j;
                break;

            }
        }
        if (clone!=-1)
        {
            new_ind[i/3] = clone/3;
            //new_ind[i+1] = clone+1;
            //new_ind[i+2] = clone+2;
        }
        else
        {
            new_ind[i/3] = positions.size()/3;
            positions.push_back(positions2[i]);
            normals.push_back(normals2[i]);
            positions.push_back(positions2[i+1]);
            normals.push_back(normals2[i+1]);
            positions.push_back(positions2[i+2]);
            normals.push_back(normals2[i+2]);
        }
        
    }
    for (int i=0;i<positions2.size()/3;i++)
    {
        //fprintf(stderr,"%d) %d\n",i,new_ind[i]);
    }
    for (int i=0;i<indices.size();i++)
    {
        indices[i] = new_ind[indices[i]];
    }
    //fprintf(stderr,"pos ready %d\n",positions.size());
    //fprintf(stderr,"norm ready %d\n",normals.size());
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
        //fprintf(stderr,"%d %d %d\n",edges[i].a,edges[i].b,edges[i].c);
    }
    vector<uint32_t> adj_ind;
    int err_count = 0;
    for (int i=0;i<sz;i++)
    {
        for (int j=0;j<=sz;j++)
        {
            if (j==sz)
            {
                //fprintf(stderr,"\n%d %d %d\n",edges[i].a,edges[i].b,edges[i].c);
                //throw(std::logic_error("Given model is not a closed body"));
                err_count++;
                break;
            }
            if ((edges[j].a==edges[i].b)&&(edges[j].b==edges[i].a))
            {
                adj_ind.push_back(edges[i].a);
                adj_ind.push_back(edges[j].c);
                break;
            }
        }
    }
    if (err_count) fprintf(stderr,"found %d errors in model",err_count);
    for (uint32_t ai : adj_ind)
    {
       //printf("%d,",(int)ai);
    }
  vector<float> norm{};
  vector<float> texc{};
  auto m = new PrimitiveMesh(positions, norm, texc, adj_ind, -1,  -1, "test_occ");
  //auto m = CreateAdjacencyTestMesh();
  return(m);
}
namespace renderer 
{


void PenumbraShadowPass::Init2(int w,int h)
{

    texWidth = w;
    texHeight = h;

    glGenTextures(1, &shadeTex);
    glBindTexture(GL_TEXTURE_2D, shadeTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, texWidth, texHeight, 0, GL_RG, GL_FLOAT, nullptr);
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
void PenumbraShadowPass::SetShaders(ShaderProgram *first, ShaderProgram *second, ShaderProgram *third,
                                    ShaderProgram *fourth, ShaderProgram *fifth)
{
    First = first;
    Second = second;
    Third = third;
    Fourth = fourth;
    Fifth = fifth;
}
void PenumbraShadowPass::Render(float4x4 view, float4x4 projection, float3 campos)
{
    static float3 lightpos;

    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glViewport(0, 0, texWidth, texHeight);
    glEnable(GL_STENCIL_TEST);  
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glStencilMask(0xFF);
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
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
        uniforms.model = glGetUniformLocation(shader->GetProgram(),"model");
        float4x4 model;
        model.identity();
        for(auto nodeId : GetScene()->rootNodes){
                drawNode(nodeId, model);
            }
        GL_CHECK_ERRORS;
        shader->StopUseShader();
        glClearColor(0.f, 0.0f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        for (int i=0;i<lights.size();i++)
        {
            glClear(GL_STENCIL_BUFFER_BIT);
            
            lightpos=lights[i].GetEffectivePos();
            float len=max_shadow_len[i];
            float radius = lights[i].GetEffectiveRadius();

            //draw shadow volumes
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
            glDepthMask(GL_FALSE);
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
            shader->SetUniform("sc",len);
            for(auto obj : occluders)
            {   
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
                for(unsigned i = 0; i < obj->GetMeshCount(); i++)
                {
                    shader->SetUniform("model", obj->GetTransform(i));
                    ((SimpleSceneObject *)obj)->DrawAdjacency(0);
                    
                }
            }
            GL_CHECK_ERRORS;
            shader->StopUseShader();

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_EQUAL);
            glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
            glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
            
            //draw a part of scene that is in full shadow
            shader = Fourth;
            shader->StartUseShader();
            shader->SetUniform("projection", projection);
            shader->SetUniform("view", view);
            shader->SetUniform("light",lightpos);
            shader->SetUniform("sc",len);
            shader->SetUniform("dist",(float)3.0);
            uniforms.model = glGetUniformLocation(shader->GetProgram(),"model");
            float4x4 model;
            model.identity();
            for(auto nodeId : GetScene()->rootNodes){
                    drawNode(nodeId, model);
                }
            GL_CHECK_ERRORS;
            GL_CHECK_ERRORS;
            shader->StopUseShader();
            glDepthFunc(GL_LEQUAL);


            //draw penumbra wedges
            glClear(GL_STENCIL_BUFFER_BIT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
            glDepthMask(GL_FALSE);

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
            shader->SetUniform("light_radius",radius);
            shader->SetUniform("sc",len);
            for(auto obj : occluders)
            {   
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
            glDepthFunc(GL_EQUAL);
            glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
            glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
            shader = Fifth;
            shader->StartUseShader();
            shader->SetUniform("projection", projection);
            shader->SetUniform("view", view);
            shader->SetUniform("light",lightpos);
            shader->SetUniform("sc",len);
            shader->SetUniform("dist",(float)3.0);
            uniforms.model = glGetUniformLocation(shader->GetProgram(),"model");
            model.identity();
            for(auto nodeId : GetScene()->rootNodes){
                    drawNode(nodeId, model);
                }
            GL_CHECK_ERRORS;
            GL_CHECK_ERRORS;
            shader->StopUseShader();
            glDepthFunc(GL_LEQUAL);
    
        }
    glEnable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void PenumbraShadowPass::drawNode(int nodeId, const float4x4& model){
    const auto& node = GetScene()->nodes[nodeId];
    float4x4 newmvp = mul(model, node.matrix);
    if(node.meshId >= 0){
        for(const auto& prim : GetScene()->meshes[node.meshId].primitives){
            draw(prim, newmvp);
        }
    }
    for(int childId : node.children){
        drawNode(childId, newmvp);
    }
}

void PenumbraShadowPass::draw(const Mesh::Primitive& prim, const float4x4 &model){
    glUniformMatrix4fv(uniforms.model, 1, GL_TRUE, model.L());
    GL_CHECK_ERRORS;
    glBindVertexArray(prim.vao);
    GL_CHECK_ERRORS;
    if(prim.hasIndexies){
        assert(prim.vao != 0);
        assert(prim.count != 0); 
        assert(prim.count % 3 == 0);
        glDrawElements(prim.mode, prim.count, prim.type, prim.indexOffset);
        GL_CHECK_ERRORS;
    } else {
        glDrawArrays(prim.mode, 0, prim.count);
    }
    GL_CHECK_ERRORS;
}

void PenumbraShadowPass::OnInit()
{
    Init2(800,600);
    fprintf(stderr,"shade = %d",shadeTex);
    if(!RegisterTexture(shadeTex, "Shade")){
        throw(std::runtime_error("AddTextureName"));
    }
    if(!RegisterTexture(depthStencilTex, "DepthStencil")){
        throw(std::runtime_error("AddTextureName"));
    }
    SetShaders(GetRenderer()->GetShader("depth_draw_s"),
               GetRenderer()->GetShader("shadow_volumes_s"),
               GetRenderer()->GetShader("penumbra_wedges_s"),
               GetRenderer()->GetShader("stencil_test_s"),
               GetRenderer()->GetShader("penumbra_drawer_s"));

  float4x4 a;
  PrimitiveMesh  *m;
  SimpleSceneObject *test_occ;
  occluders.resize(0);
  for (int i=0;i<9;i++)
  {
      //if (i&&(i%2==0)) continue;
    a = translate4x4(float3(0.001,-0.002,0.001));
    auto str = "s"+std::to_string(i)+".txt";
    m =(PrimitiveMesh *)calculateAdjacencyData2(str.c_str());
    test_occ =  new SimpleSceneObject(m,a);
    occluders.push_back(test_occ);
  }

  auto ls = GetScene()->lights.SVLights;
  for (int i=0;i<ls.GetCount();i++)
  {
      auto l = ls.Get(i);
      if (l.type==l.POINT) continue;
      lights.push_back(l);
      max_shadow_len.push_back(l.shadow_len);
      light_radius.push_back(l.GetEffectiveRadius());
  }
}
void PenumbraShadowPass::Process()
{
    //lights[0].position = GetScene()->sceneCamera.pos+float3(0,0.3,1.001);
    Render(GetScene()->sceneCamera.view,GetScene()->sceneCamera.projection, GetScene()->sceneCamera.position);
}
}