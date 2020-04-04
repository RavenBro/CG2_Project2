#include "Scene.h"
#include "ShadowMap.h"
#include "Cubemap.h"

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
    models.emplace("tank_model2", new GLTFModel("assets/Tank_M60A1_opt/tank_m60a1.gltf"));
    shaders.emplace("default_s", new ShaderProgram ({
        {GL_VERTEX_SHADER, "shaders/default/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/default/fragment.glsl"}
    }));
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

    shaders.emplace("bloom_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/bloom/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/bloom/fragment.glsl"}
    }));

    shaders.emplace("vsm_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/vsm/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/vsm/fragment.glsl"}
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
    GL_CHECK_ERRORS;

    gbuff.Init(800, 600);
    pbuff.Init(800, 600);
    rbuff.Init(800, 600);

    objects.emplace("plane", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(translate4x4(float3(-30, 0.0, -20)), scale4x4(float3(100, 100, 100)))
    ));

    objects.at("plane")->SetMaterial(PBRMaterial{0.0, 0.95, 0});

    objects.emplace("mirror", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(rotate_X_4x4(-0.2),mul(translate4x4(float3(0, -0.15, 2)), scale4x4(float3(1.0, 1.0, 1.0))))
    ));

    objects.at("mirror")->SetMaterial(PBRMaterial{1, 0.5, 0.95});

    auto tank = new ModelSceneObject(models["tank_model"].get());
    tank->SetTransform(rotate_Y_4x4(0.3), "Head");
    objects.emplace("tank", tank);
    objects["tank"]->SetMaterial(PBRMaterial{0.9, 0.4,0});

    objects.emplace("screen", new SimpleSceneObject(meshes["screen_mesh"].get()));
    objects.emplace("skybox", new SimpleSceneObject(meshes["skybox_mesh"].get()));

    vsm =  new VarianceShadowMap(800, 600);
    csm = new CascadedShadowMap(800,600);
    lightdir = float3(0.f, 5.f, 5.f);
    light = mul(
    ortho_matr(-10, 10, -10, 10, 0.0, 20.f), 
    transpose4x4(lookAtTransposed(lightdir, float3(0, 0, 0), float3(0, 1, 0))));
    
}

void Scene::SetRenderQueues()
{
    

    asmRQ.SetShaderProg(shaders.at("assemble_s").get());
    asmRQ.SetTextures(pbuff.GetLightTex(), rbuff.GetReflectionsTex(), gbuff.GetMaterialTex(),
                      gbuff.GetBloomTex(), gbuff.GetSSAOTex(), gbuff.GetShadedTex());
    asmRQ.AddSceneObject(objects["screen"].get());

    pbrRQ.SetShaderProg(shaders["pbr_lighting_s"].get());
    pbrRQ.SetTextures(gbuff.GetColorTex(), gbuff.GetNormalTex(), gbuff.GetVertexTex(), gbuff.GetMaterialTex());
    pbrRQ.SetLightSrc(0, float3(0.f, 50.f, 50.f), float3(4000.f, 4000.f, 3000.f));
    pbrRQ.SetLightSrc(1, float3(5.f, 2.f, 0.f), float3(0.f, 0.f, 0.f));
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
    gbuffRQ.SetShadowsTextures(vsm->GetTexture(),csm->GetCascade(0),csm->GetCascade(1),csm->GetCascade(2));
    gbuffRQ.AddSceneObject(objects["plane"].get());
    gbuffRQ.AddSceneObject(objects["tank"].get());
    gbuffRQ.AddSceneObject(objects.at("mirror").get());

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

    texRQ.SetShaderProg(shaders["texture_draw_s"].get());
    texRQ.SetTexture(gbuff.GetBloomTex());
    texRQ.AddSceneObject(objects["screen"].get());

    VSMRQ.SetShaderProg(shaders["vsm_s"].get());
    VSMRQ.setVSM(vsm);
    VSMRQ.SetLight(light);
    VSMRQ.AddSceneObject(objects["plane"].get());
    VSMRQ.AddSceneObject(objects["tank"].get());

    CSMRQ.SetShaderProg(shaders["vsm_s"].get());
    CSMRQ.SetLight(lookAtTransposed(lightdir, float3(0, 0, 0), float3(0, 1, 0)));
    //CSMRQ.SetLight(transpose4x4(camera->getViewMatrix()));
    CSMRQ.setCSM(csm);
    CSMRQ.AddSceneObject(objects["plane"].get());
    CSMRQ.AddSceneObject(objects["tank"].get());
    GL_CHECK_ERRORS;
}

void Scene::Render()
{
    static long tick=0;
    if (tick%SHADOWS_RENDER_DELAY==0)
    {
        CSMRQ.SetCurCamera(transpose4x4(camera->getViewMatrix()));
        csm->BindTarget();
        CSMRQ.Draw();
        csm->Unbind(800,600);
        blurRQ.Blur(csm->GetCascade(0),0);
        blurRQ.Blur(csm->GetCascade(1),0);
        blurRQ.Blur(csm->GetCascade(2),0);
        vsm->BindTarget();
        VSMRQ.Draw();
        vsm->Unbind(800,600);
    }
    gbuff.Bind();
    gbuff.SetRenderState();
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
    gbuff.Clear();
    gbuffRQ.SetView(camera->getViewMatrix());
    gbuffRQ.SetLights(light,CSMRQ.GetLights(),CSMRQ.GetCasZBorders(projection));
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

    

    glViewport(0, 0, 800, 600);
    glClearColor(0.f, 1.0f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    asmRQ.Draw();
    //texRQ.Draw();
    tick++;
}
Scene::~Scene()
{
    delete(vsm);
    delete(csm);
}