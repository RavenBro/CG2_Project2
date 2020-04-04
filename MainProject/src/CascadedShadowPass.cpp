
#include "CascadedShadowPass.hpp"
namespace renderer
{
float *CascadedShadowPass::GetCasZBorders(float4x4 projection)
    {
        for (int i = 0 ; i < 3 ; i++) 
        {
            float4 vView(0.0f, 0.0f, m_cascadeEnd[i + 1], 1.0f);
            float4 vClip = mul(projection,vView);
            casZborders[i]=-vClip.z;
        }
        //printf("borders %f %f %f \n",casZborders[0]   ,casZborders[1],casZborders[2]);
        return(casZborders);
    }
void CascadedShadowPass::CalcOrthoProjs()
    {
        float4x4 caminv = inverse4x4(transpose4x4(cameraViewTransposed));

      float ar = 600.0 / 800.0;
      float FOV = M_PI/2.0;
      float tanHalfHFOV = tanf(FOV/ 2.0f);
      float tanHalfVFOV = tanf((FOV * ar) / 2.0f);
      for (uint i = 0 ; i < 3 ; i++) 
      {
            float xn = m_cascadeEnd[i]     * tanHalfHFOV;
            float xf = m_cascadeEnd[i + 1] * tanHalfHFOV;
            float yn = m_cascadeEnd[i]     * tanHalfVFOV;
            float yf = m_cascadeEnd[i + 1] * tanHalfVFOV;
 
            float4 frustumCorners[8] = {
                  // Ближняя плоскость
                  float4(xn,   yn, m_cascadeEnd[i], 1.0),
                  float4(-xn,  yn, m_cascadeEnd[i], 1.0),
                  float4(xn,  -yn, m_cascadeEnd[i], 1.0),
                  float4(-xn, -yn, m_cascadeEnd[i], 1.0),
 
                  // Дальняя плоскость
                  float4(xf,   yf, m_cascadeEnd[i + 1], 1.0),
                  float4(-xf,  yf, m_cascadeEnd[i + 1], 1.0),
                  float4(xf,  -yf, m_cascadeEnd[i + 1], 1.0),
                  float4(-xf, -yf, m_cascadeEnd[i + 1], 1.0)
            };
            float4 frustumCornersL[8];
            float minX = 10000;
            float maxX = -10000;
            float minY = 10000;
            float maxY = -10000;
            float minZ = 10000;
            float maxZ = -10000;
            
            for (uint j = 0 ; j < 8 ; j++) 
            {
                // Преобразуем координаты усеченоой пирамиды из пространства камеры в мировое пространство
                
                float4 vW = mul(caminv,frustumCorners[j]);
                // И ещё раз из мирового в пространство света
                frustumCornersL[j] = mul(transpose4x4(lightViewTransposed),vW);
            
                minX = fmin(minX, frustumCornersL[j].x);
                maxX = fmax(maxX, frustumCornersL[j].x);
                minY = fmin(minY, frustumCornersL[j].y);
                maxY = fmax(maxY, frustumCornersL[j].y);
                minZ = fmin(minZ, frustumCornersL[j].z);
                maxZ = fmax(maxZ, frustumCornersL[j].z);
            }
            m_shadowOrthoProjInfo[i]=ortho_matr(minX-1,maxX+1,minY-1,maxY+1,-maxZ-1,-minZ+1);
            
      }
    }
float4x4 CascadedShadowPass::ortho_matr(float left, float right, float bot, float top, float near, float far)
{
  float4x4 result;
  result.set_row(0, float4(2/(right - left), 0.f, 0.f, -(right + left)/(right - left)));
  result.set_row(1, float4(0.f, 2/(top - bot), 0.f, -(top+bot)/(top - bot)));
  result.set_row(2, float4(0.f, 0.f, -2/(far - near), -(far + near)/(far-near)));
  result.set_row(3, float4(0.f, 0.f, 0.f, 1.f));
  return result;
}
void CascadedShadowPass::Init2(int w,int h)
{
    texWidth = w;
    texHeight = h;

    depthTex = GenerateTexture2D(DepthTexFormat(), texWidth, texHeight, 0);
    if(!depthTex) throw(std::runtime_error("CascadedShadowPass : texture gen error"));
    if(!RegisterTexture(depthTex, "Depth")){
        throw(std::runtime_error("AddTextureName"));}
    for (int i=0;i<3;i++)
    {
        cascadeTex[i] = GenerateTexture2D(SMTexFormat(), texWidth, texHeight, 0);
        if(!cascadeTex[i]) throw(std::runtime_error("CascadedShadowPass : texture gen error"));
        if(!RegisterTexture(cascadeTex[i], "cascadeTex_"+std::to_string(i))){
            throw(std::runtime_error("AddTextureName"));}
    }
    glGenFramebuffers(1, &FrameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cascadeTex[0], 0);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cascadeTex[0], 0);
    glDrawBuffer(GL_NONE);
    /*if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        
        throw(std::runtime_error("CascadedShadowPass: framebuffer incomplete"));
    }*/
}
void CascadedShadowPass::drawNode(int nodeId, const float4x4& model){
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

void CascadedShadowPass::draw(const Mesh::Primitive& prim, const float4x4 &model){
    glUniformMatrix4fv(uniforms.MVP, 1, GL_TRUE, model.L());
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
void CascadedShadowPass::OnInit()
{
    SetShader("shadowmap_s");
    BindShader();
    uniforms.MVP = TryGetUniformLoc("MVP");
    UnbindShader();
    Init2(800,600);
    lightViewTransposed = lookAtTransposed(
        float3(-3, 6, -7), float3(-4, 5, -7), float3(0, 1, 0));
}
void CascadedShadowPass::Process()
{
    
    static int iter = -1;
    iter++;
    if (iter%4) return;
    cameraViewTransposed = transpose4x4(GetScene()->sceneCamera.view);
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cascadeTex[0], 0);
    glDrawBuffer(GL_NONE);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cascadeTex[0], 0);
    /*if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        fprintf(stderr,"<error %d %d %d>\n",(int)glCheckFramebufferStatus(GL_FRAMEBUFFER),depthTex,cascadeTex[0]);
        throw(std::runtime_error("CascadedShadowPass: framebuffer incomplete"));
    }*/
    glViewport(0, 0, texWidth, texHeight);
    CalcOrthoProjs();
    glCullFace(GL_FRONT);
    //glDisable(GL_CULL_FACE);
    for (int i=0;i<3;i++)
    {
        cascades[i]=mul(m_shadowOrthoProjInfo[i],transpose4x4(lightViewTransposed));
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cascadeTex[i], 0);
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        auto vsm_s = GetShader();
        vsm_s->StartUseShader();
        /*int a =0;
        for(auto nodeId : GetScene()->rootNodes)
        {
            a++;
           if (a!=5) continue;
           
            drawNode(nodeId, cascades[i]);
        }*/
        drawNode(GetScene()->rootNodes[4], cascades[i]);
        auto t = GetScene()->tank;
        for (int j=0;j< t->GetMeshCount();j++)
        {
            auto tr = t->GetTransform(j);
            vsm_s->SetUniform("MVP",mul(cascades[i],tr));
            t->Draw(j);
        }
        //fprintf(stderr,"cascades %d\n",a);
        vsm_s->StopUseShader();
    }
    //glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
}