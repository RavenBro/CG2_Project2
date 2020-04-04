/*#include "IndoorScene.h"
#include "ShadowMap.h"
#include "Cubemap.h"

void IndoorScene::LoadData()
{
    meshes.emplace("plane_mesh", CreateSimplePlaneMesh());
    meshes.emplace("wall", CreateSimpleBoxMesh());
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
    shaders.emplace("default_s", new ShaderProgram ({
        {GL_VERTEX_SHADER, "shaders/default/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/default/fragment.glsl"}
    }));
    shaders.emplace("texture_draw_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/vsm_buff_draw/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/vsm_buff_draw/fragment.glsl"}
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

    shaders.emplace("bloom_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/bloom/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/bloom/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;
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
    shaders.emplace("hbao_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/HBAO/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/HBAO/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;

    gbuff.Init(800, 600);
    pbuff.Init(800, 600);
    rbuff.Init(800, 600);

    objects.emplace("plane", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(translate4x4(float3(-30, 0.0, -20)), scale4x4(float3(100, 100, 100)))
    ));
    objects.emplace("wall1", new SimpleSceneObject(
        meshes["wall"].get(),
        mul(translate4x4(float3(0.0, -0.1, -8.0)), scale4x4(float3(10, 8, 1)))
    ));
    objects.emplace("wall2", new SimpleSceneObject(
        meshes["wall"].get(),
        mul(translate4x4(float3(0.0, -0.1, 8.0)), scale4x4(float3(10, 8, 1)))
    ));
    objects.emplace("wall3", new SimpleSceneObject(
        meshes["wall"].get(),
        mul(rotate_Y_4x4(M_PI/2), mul(translate4x4(float3(0.0, -0.1, -5.0)), scale4x4(float3(16, 8, 1))))
    ));
    objects.emplace("wall4", new SimpleSceneObject(
        meshes["wall"].get(),
        mul(rotate_Y_4x4(M_PI/2),mul(translate4x4(float3(0.0, -0.1, -5.0)), scale4x4(float3(16, 8, 1))))
    ));
    objects.emplace("wall5", new SimpleSceneObject(
        meshes["wall"].get(),
        mul(rotate_X_4x4(M_PI/2),mul(translate4x4(float3(0.0, 0.0, -10.0)), scale4x4(float3(10, 16, 1))))
    ));
    objects.at("plane")->SetMaterial(PBRMaterial{0.0, 0.95, 0});
    objects.at("wall1")->SetMaterial(PBRMaterial{0.1,0.9,0.0});
    objects.at("wall2")->SetMaterial(PBRMaterial{0.1,0.9,0.0});
    objects.at("wall3")->SetMaterial(PBRMaterial{0.1,0.9,0.0});
    objects.at("wall4")->SetMaterial(PBRMaterial{0.1,0.9,0.0});
    objects.at("wall5")->SetMaterial(PBRMaterial{0.1,0.9,0.0});
    objects.emplace("mirror", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(rotate_X_4x4(-0.2),mul(translate4x4(float3(0, -0.15, 2)), scale4x4(float3(1.0, 1.0, 1.0))))
    ));

    objects.at("mirror")->SetMaterial(PBRMaterial{1, 0.5, 0.95});

    auto tank = new ModelSceneObject(models["tank_model"].get());
    tank->SetTransform(rotate_Y_4x4(0.3), "Head");
    objects.emplace("tank", tank);
    objects["tank"]->SetMaterial(PBRMaterial{0.9, 0.4,0});

    auto tank2 = new ModelSceneObject(models["tank_model"].get());
    for (int i=0;i<tank2->GetMeshCount();i++)
    {
        tank2->SetTransform(translate4x4(float3(0.0,0.0,-3.0)),i);
    }
    objects.emplace("tank2", tank2);
    objects["tank2"]->SetMaterial(PBRMaterial{0.9, 0.4,0.8});

    auto tank3 = new ModelSceneObject(models["tank_model"].get());
    for (int i=0;i<tank3->GetMeshCount();i++)
    {
        tank3->SetTransform(translate4x4(float3(0.0,0.0,-6.0)),i);
    }
    objects.emplace("tank3", tank3);
    objects["tank3"]->SetMaterial(PBRMaterial{0.0, 0.9,0.0});

    objects.emplace("screen", new SimpleSceneObject(meshes["screen_mesh"].get()));
    objects.emplace("skybox", new SimpleSceneObject(meshes["skybox_mesh"].get()));

    
}

void IndoorScene::SetRenderQueues()
{
    

    asmRQ.SetShaderProg(shaders.at("assemble_s").get());
    asmRQ.SetTextures(pbuff.GetLightTex(), rbuff.GetReflectionsTex(), gbuff.GetMaterialTex(),
                      gbuff.GetBloomTex(), gbuff.GetSSAOTex(), gbuff.GetShadedTex());
    asmRQ.AddSceneObject(objects["screen"].get());

    pbrRQ.SetShaderProg(shaders["pbr_lighting_s"].get());
    pbrRQ.SetTextures(gbuff.GetColorTex(), gbuff.GetNormalTex(), gbuff.GetVertexTex(), gbuff.GetMaterialTex());
    pbrRQ.SetLightSrc(0, float3(0.f, 5.f, 5.f), float3(10.f, 10.f, 10.f));
    pbrRQ.SetLightSrc(1, float3(0.f, 5.f, -5.f), float3(20.f, 0.f, 0.f));
    pbrRQ.SetLightSrc(2, float3(0.f, 2.f, 5.f), float3(0.f, 0.f, 00.f));
    pbrRQ.SetLightSrc(3, float3(0.f, -5.f, -5.f), float3(0.f, 0.f, 0.f));
    pbrRQ.AddSceneObject(objects["screen"].get());

    ssrRQ.SetShaderProg(shaders.at("reflect_s").get());
    ssrRQ.SetTextures(pbuff.GetLightTex(), gbuff.GetNormalTex(), 
        gbuff.GetDepthTex(), gbuff.GetVertexTex(), gbuff.GetMaterialTex());
    ssrRQ.SetProj(projection); 
    ssrRQ.AddSceneObject(objects["screen"].get());

    gbuffRQ.SetShaderProg(shaders["gbuffer_s"].get());
    gbuffRQ.SetProj(projection);
    gbuffRQ.SetShadows(0);
    gbuffRQ.AddSceneObject(objects["plane"].get());
    gbuffRQ.AddSceneObject(objects["tank"].get());
    gbuffRQ.AddSceneObject(objects["tank2"].get());
    gbuffRQ.AddSceneObject(objects["tank3"].get());
    gbuffRQ.AddSceneObject(objects.at("mirror").get());
    gbuffRQ.AddSceneObject(objects["wall1"].get());
    gbuffRQ.AddSceneObject(objects["wall2"].get());
    gbuffRQ.AddSceneObject(objects["wall3"].get());
    gbuffRQ.AddSceneObject(objects["wall4"].get());
    gbuffRQ.AddSceneObject(objects["wall5"].get());

    lightRQ.SetShaderProg(shaders["ssao_s"].get());
    lightRQ.InitSSAO(gbuff.GetDepthTex(), gbuff.GetNormalTex(), gbuff.GetVertexTex(),
         gbuff.GetNoiseTex(), pbuff.GetLightTex(), projection);
    lightRQ.AddSceneObject(objects["screen"].get());

    blurRQ.Init(800, 600, gbuff.GetSSAOTex());
    blurRQ.SetShaderProg(shaders["blur_s"].get());
    blurRQ.AddSceneObject(objects["screen"].get());

    bloomRQ.SetShaderProg(shaders["bloom_s"].get());
    bloomRQ.SetTexture(gbuff.GetShadedTex());        
    bloomRQ.AddSceneObject(objects["screen"].get());

    HBAORQ.SetShaderProg(shaders["hbao_s"].get());
    HBAORQ.Init(800,600,gbuff.GetVertexTex(),gbuff.GetNoiseTex());
    HBAORQ.AddSceneObject(objects["screen"].get());

    texRQ.SetShaderProg(shaders["texture_draw_s"].get());
    texRQ.SetTexture(rbuff.GetReflectionsTex());
    texRQ.AddSceneObject(objects["screen"].get());

    
    GL_CHECK_ERRORS;
}

void IndoorScene::Render()
{

    gbuff.Bind();
    gbuff.SetRenderState();
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
    gbuff.Clear();
    gbuffRQ.SetView(camera->getViewMatrix());
    gbuffRQ.Draw();

    gbuff.SetLightingState();
    lightRQ.Draw();
    blurRQ.Blur(gbuff.GetBloomTex(),1);
    blurRQ.Blur(gbuff.GetSSAOTex(),1);
    blurRQ.Blur(gbuff.GetShadedTex(),1);
    gbuff.Unbind();

    pbuff.Bind();
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    pbrRQ.SetView(camera->getViewMatrix());
    pbrRQ.Draw();
    pbuff.Unbind();

    rbuff.Bind();
    ssrRQ.Draw();
    blurRQ.Blur(rbuff.GetReflectionsTex(),0);
    rbuff.Unbind();

    
    HBAORQ.Draw();
    blurRQ.Blur(HBAORQ.GetHBAOTex(),0);
    glViewport(0, 0, 800, 600);
    glClearColor(0.f, 1.0f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //asmRQ.Draw();
    texRQ.Draw();
}
IndoorScene::~IndoorScene()
{

}*/