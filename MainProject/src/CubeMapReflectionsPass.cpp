#include "CubeMapReflectionsPass.hpp"
#include "framework/Objects3d.h"
#include "framework/Primitives.h"
#include "framework/HelperGL.h"
#include "Renderer.hpp"
using namespace std;
using namespace LiteMath;
namespace renderer
{
    static void Show_matr(float4x4 matr) {
      for (int i = 0; i < 4; i++)
      {
          std::cout<<matr.row[i].x<<" ";
          std::cout<<matr.row[i].y<<" ";
          std::cout<<matr.row[i].z<<" ";
          std::cout<<matr.row[i].w<<" ";
          std::cout<<std::endl;
      }
      
  }
void CubeMapReflectionsPass::Init2(int w, int h, float3 center)
{
    texHeight = h;
    texWidth = w;
    this->center = center;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    for (int i = 0; i < 6; i++)
    { 
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA,
        GL_FLOAT, 0);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &resTex);
    glBindTexture(GL_TEXTURE_2D, resTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &FrameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    for (int i = 0; i < 6; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                tex, 0);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    //glReadBuffer(GL_NONE);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    GL_CHECK_ERRORS;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void CubeMapReflectionsPass::Render()
{
   
    for (int i = 0; i < 6; i++)
    {
          //if(i==2) continue;
        /*
        GL_TEXTURE_CUBE_MAP_POSITIVE_X  Right       0
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X  Left        1
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y  Top     2
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y  Bottom  3
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z  Back        4
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  Front       5
        */
        
        /* Выставляется камера для нужной грани */
        float4x4 ViewMatrix;
        float4x4 ProjectionMatrix = transpose4x4(projectionMatrixTransposed(
            90, (float)texWidth/(float)texHeight,0.3,100
        ));
        float3 pos =center;
        switch (i)
        {
        case 0:
            /*ViewMatrix=transpose4x4(lookAtTransposed(
                float3(-1,0,0),float3(0.0,0.5,0.0),float3(0,-1,0)
            ));*/
            ViewMatrix=transpose4x4(lookAtTransposed(
                pos,pos+float3(1.0,0.0,0.0),float3(0,-1,0)
            ));
            break;
        case 1:
            ViewMatrix=transpose4x4(lookAtTransposed(
                pos,pos+float3(-1,0,0),float3(0,-1,0)
            ));
            break;
        case 2:
            ViewMatrix=transpose4x4(lookAtTransposed(
                pos,pos+float3(0,1,0),float3(0,0,1)
            ));
            break;
        case 3:
            ViewMatrix=transpose4x4(lookAtTransposed(
               pos,pos+float3(0,-1,0),float3(0,0,-1)
            ));
            break;
        case 4:
            ViewMatrix=transpose4x4(lookAtTransposed(
                pos,pos+float3(0,0,1),float3(0,-1,0)
            ));
            break;
        case 5:
            ViewMatrix=transpose4x4(lookAtTransposed(
                pos,pos+float3(0,0,-1),float3(0,-1,0)
            ));
        default:
            break;
        };
        int4 prev = GetRenderer()->GetScreenViewport();
        //GetRenderer()->SetViewport(int4(0,0,texWidth,texHeight));
        float4x4 pr = GetScene()->sceneCamera.projection;
        if (true)
        {
            GetScene()->sceneCamera.projection = transpose(projectionMatrixTransposed
            (90,1,0.1,1000));
        }
        GetScene()->SetCamera(ViewMatrix,center);
        GetRenderer()->Draw();
        GetRenderer()->SetViewport(prev);
        GetScene()->sceneCamera.projection = pr;
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
        glViewport(0, 0, texWidth, texHeight);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
        glDrawBuffer(GL_COLOR_ATTACHMENT0 +i);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear( GL_DEPTH_BUFFER_BIT);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i , 
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex, 0);

        GL_CHECK_ERRORS;
        
        
        targetTex = GetRenderer()->GetTexture("Asm/Result");
        auto quadMesh = GetRenderer()->GetScreenQuad();
        BindShader();
        auto loc = GetUniformLoc("vsm_tex");
        if(loc == -1){
            throw(std::runtime_error("TextureDrawer::Init : vsm_tex location not found"));
        }
        glUniform1i(loc, 0);
        UnbindShader();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, targetTex);
        BindShader();

        
        quadMesh->Draw();
        UnbindShader();
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    auto m = CreateSimplePlaneMesh();
    auto matr = mul(rotate_Z_4x4(M_PI_2),
    mul(scale4x4(float3(0.2,1,0.2)),translate4x4(float3(5*0.916,-1.44,5*0.386))));
    float3 res = mul(matr,float3(0,0,0));
    fprintf(stderr,"<%f %f %f>\n",res.x,res.y,res.z);
    obj = new SimpleSceneObject(m,matr);
    
}
void CubeMapReflectionsPass::drawNode(int nodeId, const float4x4& model){
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

void CubeMapReflectionsPass::draw(const Mesh::Primitive& prim, const float4x4 &model){
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
void CubeMapReflectionsPass::OnInit()
{
    SetShader(GetRenderer()->GetShader("textureDraw_s"));
    Drawer = GetRenderer()->GetShader("CMReflections_s");
    Init2(600,600,float3(1.447,0.916,0.386));
    //  Init2(600,600,float3(0.5,2.916,0.5));
    if(!RegisterTexture(tex, "Cubemap")){
        throw(std::runtime_error("AddTextureName"));
    }
    if(!RegisterTexture(resTex, "Res")){
        throw(std::runtime_error("AddTextureName"));
    }
    
    obj = nullptr;
}
void CubeMapReflectionsPass::Process()
{
    if (!obj) return;
    glDisable(GL_CULL_FACE);
    Drawer->StartUseShader();
    Drawer->SetUniform("cubemap",16);
    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    Drawer->SetUniform("campos",GetScene()->sceneCamera.position);
    Drawer->SetUniform("view",GetScene()->sceneCamera.view);
    Drawer->SetUniform("projection",GetScene()->sceneCamera.projection);
    for (int i=0;i<obj->GetMeshCount();i++)
    {
        Drawer->SetUniform("model",obj->GetTransform(i));
        obj->Draw(i);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glEnable(GL_CULL_FACE);
    Drawer->StopUseShader();
}
}