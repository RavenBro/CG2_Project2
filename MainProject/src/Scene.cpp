#include "Scene.h"
#include "ShadowMap.h"
#include "Cubemap.h"

inline float4x4 ortho(float left, float right, float bot, float top, float near, float far)
{
  float4x4 result;
  result.set_row(0, float4(2/(right - left), 0.f, 0.f, -(right + left)/(right - left)));
  result.set_row(1, float4(0.f, 2/(top - bot), 0.f, -(top+bot)/(top - bot)));
  result.set_row(2, float4(0.f, 0.f, -2/(far - near), -(far + near)/(far-near)));
  result.set_row(3, float4(0.f, 0.f, 0.f, 1.f));
  return result;
}

void Scene::LoadData()
{
    meshes.emplace("plane_mesh", CreateSimplePlaneMesh());
    meshes.emplace("screen_mesh", CreateQuad());
    meshes.emplace("skybox_mesh", new Skybox());;
        
    auto sb = (Skybox*)meshes["skybox_mesh"].get();
    if(sb->Load(vector<std::string> {
        "assets/Skybox/hills_bk.tga",
        "assets/Skybox/hills_dn.tga",
        "assets/Skybox/hills_ft.tga",
        "assets/Skybox/hills_lf.tga",
        "assets/Skybox/hills_rt.tga",
        "assets/Skybox/hills_up.tga"
    }) == false)
    {
        throw std::logic_error("Skybox not loaded\n");
    }

    models.emplace("tank_model", new GLTFModel("assets/Tank_M60A1_opt/tank_m60a1.gltf"));
    
    shaders.emplace("texture_draw_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/vsm_buff_draw/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/vsm_buff_draw/fragment.glsl"}
    }));
    shaders.emplace("skybox_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/SkyBox/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/SkyBox/fragment.glsl"}
    }));

    shaders.emplace("gbuffer_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/gbuffer/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/gbuffer/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;

    shaders.emplace("ssao_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/SSAO/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/SSAO/fragment.glsl"}
    }));

    shaders.emplace("blur_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/gaucean_blur/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/gaucean_blur/fragment.glsl"}
    }));

    shaders.emplace("pbr_lighting_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/pbr_lighting/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/pbr_lighting/fragment.glsl"}
    }));
    
    shaders.emplace("reflect_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/screen_reflection/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/screen_reflection/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;
    shaders.emplace("assemble_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/assemble/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/assemble/fragment.glsl"}
    }));

    shaders.emplace("vsm_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/vsm/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/vsm/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;
    shaders.emplace("rsm_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/rsm/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/rsm/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;

    gbuff.Init(800, 600);
    GL_CHECK_ERRORS;
    pbuff.Init(800, 600);
    GL_CHECK_ERRORS;
    rbuff.Init(800, 600);
    GL_CHECK_ERRORS;
    refSM.Init(512, 512);
    GL_CHECK_ERRORS;
    globLightSM.reset(new VarianceShadowMap(1024, 1024));
    GL_CHECK_ERRORS;
    gbuffRQ.SetShaderProg(shaders["gbuffer_s"].get());
    gbuffRQ.SetProj(projection);
    
    lightRQ.SetShaderProg(shaders["ssao_s"].get());
    lightRQ.InitSSAO(gbuff.GetDepthTex(), gbuff.GetNormalTex(), gbuff.GetVertexTex(), projection);
    GL_CHECK_ERRORS;
    blurRQ.Init(800, 600, rbuff.GetReflectionsTex(), rbuff.GetTempTex());
    blurRQ.SetShaderProg(shaders["blur_s"].get());
    blurRQ.SetSigma(3);
    

    pbrRQ.SetShaderProg(shaders["pbr_lighting_s"].get());
    pbrRQ.SetTextures(gbuff.GetColorTex(), gbuff.GetNormalTex(), gbuff.GetVertexTex(), gbuff.GetMaterialTex());
    

    asmRQ.SetShaderProg(shaders.at("assemble_s").get());
    asmRQ.SetTextures(pbuff.GetLightTex(), rbuff.GetReflectionsTex(), gbuff.GetMaterialTex());

    pbrRQ.SetLightSrc(0, float3(0.f, 2.f, 0.f), float3(0.f, 0.f, 0.f));
    pbrRQ.SetLightSrc(1, float3(5.f, 2.f, 0.f), float3(0.f, 0.f, 0.f));
    pbrRQ.SetLightSrc(2, float3(0.f, 0.46f, 1.26f), float3(0.f, 0.5f, 0.f));
    pbrRQ.SetLightSrc(3, float3(1.17f, 2.5f, 4.6f), float3(10.f, 10.f, 10.f));
    pbrRQ.SetGlobalLight(normalize(float3(0.f, -1.f, -2.f)), float3(5.f, 5.f, 5.f));

    ssrRQ.SetShaderProg(shaders.at("reflect_s").get());
    ssrRQ.SetTextures(pbuff.GetLightTex(), gbuff.GetNormalTex(), 
        gbuff.GetDepthTex(), gbuff.GetVertexTex(), gbuff.GetMaterialTex());
    ssrRQ.SetProj(projection);   

    rsmRQ.SetShaderProg(shaders.at("rsm_s").get());
    float4x4 rsmTrans = transpose(lookAtTransposed(float3(1.17, 2.5, 4.6), float3(0, 0, 0), float3(0, 1, 0)));
    rsmRQ.SetView(rsmTrans);
    rsmRQ.SetProj(transpose(projectionMatrixTransposed(
        90, 1, 0.1, 10
    )));
    rsmTrans = mul(transpose(projectionMatrixTransposed(90, 1, 0.1, 10)), rsmTrans);
    pbrRQ.SetRSM(rsmTrans, refSM.GetDepthTex(), refSM.GetNormalTex(), refSM.GetPosTex(), refSM.GetFluxTex());
    rsmRQ.SetLightColor(float3(10, 10, 10));
}

void Scene::CreateObjects()
{
    objects.emplace("plane", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        scale4x4(float3(100, 100, 100))
    ));

    objects.at("plane")->SetMaterial(PBRMaterial{0.5, 0.8, 0.5, float3{0.8, 0.8, 0.8}});

    PBRMaterial material {0.5, 0.5, 0, float3{0.8, 0, 0}};

    objects.emplace("plane_l", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(translate4x4(float3{-2.2f, 1.f, 0.f}), mul(rotate_Z_4x4(-M_PI_2), scale4x4(float3(3.f, 3.f, 3.f))))
    ));
    objects.at("plane_l")->SetMaterial(material);
    material.color = float3(0, 0.8, 0.0);

    objects.emplace("plane_r", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(translate4x4(float3{3.f, 1.f, 0.f}), mul(rotate_Z_4x4(M_PI_2), scale4x4(float3(3.f, 3.f, 3.f))))
    ));
    objects.at("plane_r")->SetMaterial(material);

    material.color = float3(0, 0.0, 0.8);
    //material.reflection = 0.4;
    objects.emplace("plane_b", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(translate4x4(float3{0, 1.f, -1.5f}), mul(rotate_X_4x4(M_PI_2), scale4x4(float3(3.f, 2.f, 3.f))))
    ));
    objects.at("plane_b")->SetMaterial(material);

    objects.emplace("mirror", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(translate4x4(float3(-1, 0.05, 2)), scale4x4(float3(0.5, 0.5, 0.5)))
    ));
    
    objects.at("mirror")->SetMaterial(PBRMaterial{0.8, 0.2, 0.8, float3(0, 0, 0)});

    objects.emplace("glossy_mirror", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(translate4x4(float3(1, 0.05, 2)), scale4x4(float3(0.5, 0.5, 0.5)))
    ));
    
    objects.at("glossy_mirror")->SetMaterial(PBRMaterial{0.8, 0.7, 0.99, float3(0.4, 0.4, 0.4)});

    auto tank = new ModelSceneObject(models["tank_model"].get());
    tank->SetTransform(rotate_Y_4x4(0.3), "Head");
    objects.emplace("tank", tank);
    objects["tank"]->SetMaterial(PBRMaterial{0.9, 0.4});

    objects.emplace("screen", new SimpleSceneObject(meshes["screen_mesh"].get()));
    objects.emplace("skybox", new SimpleSceneObject(meshes["skybox_mesh"].get()));
        
    texRQ.SetShaderProg(shaders["texture_draw_s"].get());
    vsmRQ.SetShaderProg(shaders.at("vsm_s").get());
    std::cout << gbuff.GetColorTex() << "\n";
    std::cout << pbuff.GetLightTex() << "\n";
    
    std::cout << globLightSM->GetTexture() << "\n";
    texRQ.SetTexture(refSM.GetFluxTex());
    texRQ.AddSceneObject(objects["screen"].get());
    GL_CHECK_ERRORS;

    vsmBlurRQ.Init(1024, 1024, globLightSM->GetTexture(), globLightSM->GetTempTex());
    vsmBlurRQ.SetShaderProg(shaders.at("blur_s").get());
    vsmBlurRQ.SetSigma(1.5);

    gbuffRQ.AddSceneObject(objects["plane"].get());
    gbuffRQ.AddSceneObject(objects.at("plane_l").get());
    gbuffRQ.AddSceneObject(objects.at("plane_r").get());
    gbuffRQ.AddSceneObject(objects.at("plane_b").get());
    gbuffRQ.AddSceneObject(objects["tank"].get());
    gbuffRQ.AddSceneObject(objects.at("mirror").get());
    gbuffRQ.AddSceneObject(objects.at("glossy_mirror").get());

    vsmRQ.AddSceneObject(objects["plane"].get());
    vsmRQ.AddSceneObject(objects["tank"].get());
    vsmRQ.AddSceneObject(objects.at("mirror").get());
    vsmRQ.AddSceneObject(objects.at("plane_l").get());
    vsmRQ.AddSceneObject(objects.at("plane_r").get());
    vsmRQ.AddSceneObject(objects.at("plane_b").get());

    rsmRQ.AddSceneObject(objects["plane"].get());
    rsmRQ.AddSceneObject(objects["tank"].get());
    rsmRQ.AddSceneObject(objects.at("mirror").get());
    rsmRQ.AddSceneObject(objects.at("plane_l").get());
    rsmRQ.AddSceneObject(objects.at("plane_r").get());
    rsmRQ.AddSceneObject(objects.at("plane_b").get());

    float4x4 lightView = transpose(lookAtTransposed(
        float3(0.f, 2.3f, 2.7f),
        float3(0.f, 0.f, 0.f),
        float3(0.f, 1.f, 0.f)
    ));
    float4x4 lightProj = ortho(-10.f, 10.f, -10.f, 10.f, 0.f, 10.f);
    
    vsmRQ.SetView(lightView);
    vsmRQ.SetProj(lightProj);
    
    float4x4 lightVP = mul(lightProj, lightView);
    pbrRQ.SetGlobalLightSM(globLightSM->GetTexture(), lightVP);

    lightRQ.AddSceneObject(objects["screen"].get());
    blurRQ.AddSceneObject(objects["screen"].get());
    vsmBlurRQ.AddSceneObject(objects.at("screen").get());
    pbrRQ.AddSceneObject(objects["screen"].get());
    ssrRQ.AddSceneObject(objects.at("screen").get());
    asmRQ.AddSceneObject(objects.at("screen").get());
}

void Scene::Render()
{
    globLightSM->BindTarget();
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    vsmRQ.Draw();
    globLightSM->Unbind(800, 600);
    
    vsmBlurRQ.Draw();

    refSM.BindTarget();
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    rsmRQ.Draw();
    refSM.Unbind();

    gbuff.Bind();
    gbuff.SetRenderState();
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
    gbuff.Clear();
    gbuffRQ.SetView(camera->getViewMatrix());
    gbuffRQ.Draw();
    gbuff.Unbind();

    //glGenerateTextureMipmap(gbuff.GetDepthTex());

    pbuff.Bind();
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    pbrRQ.SetView(camera->getViewMatrix());
    pbrRQ.SetInverseCam(inverse4x4(camera->getViewMatrix()));
    pbrRQ.Draw();
    pbuff.Unbind();

    rbuff.Bind();
    ssrRQ.SetInvView(inverse4x4(camera->getViewMatrix()));
    ssrRQ.Draw();
    rbuff.Unbind();

    blurRQ.Draw();
    

    glViewport(0, 0, 800, 600);
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //texRQ.Draw();
    asmRQ.Draw();
}