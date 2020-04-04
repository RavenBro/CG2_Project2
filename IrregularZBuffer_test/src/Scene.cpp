#include "Scene.h"
#include "Cubemap.h"
#include <unistd.h>
#include <sys/types.h>
ModelSceneObject *tank;
SimpleSceneObject *obs;
void Scene::LoadData()
{
    meshes.emplace("plane_mesh", CreateSimplePlaneMesh());
    meshes.emplace("wall", CreateSimpleBoxMesh());
    meshes.emplace("screen_mesh", CreateQuad());
    meshes.emplace("skybox_mesh", new Skybox());
    meshes.emplace("adj_test_mesh",CreateAdjacencyTestMesh());
    meshes.emplace("adj_test_mesh_norm",CreateSimpleBoxMesh());
    meshes.emplace("adj_test_mesh2",CreateAdjacencyTestMesh2());
    meshes.emplace("adj_test_mesh_norm2",CreateAdjacencyTestMesh2Norm());

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
    models.emplace("skull_model", new GLTFModel("assets/Tank_M60A1_opt/tank_m60a1.gltf"));

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

    shaders.emplace("hbao_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/HBAO/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/HBAO/fragment.glsl"}
    }));

    shaders.emplace("blur_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/gaucean_blur/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/gaucean_blur/fragment.glsl"}
    }));

    shaders.emplace("bloom_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/bloom/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/bloom/fragment.glsl"}
    }));

    shaders.emplace("csm_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/csm/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/csm/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;

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

    shaders.emplace("bloom_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/bloom/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/bloom/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;

    shaders.emplace("simple_depth_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/simple_depth/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/simple_depth/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;

    shaders.emplace("tank_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/tank/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/tank/fragment.glsl"}
    }));

    shaders.emplace("depth_draw_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/depth_draw/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/depth_draw/fragment.glsl"}
    }));

    shaders.emplace("stencil_test_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/stencil_test/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/stencil_test/fragment.glsl"}
    }));
    shaders.emplace("penumbra_drawer_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/penumbra_drawer/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/penumbra_drawer/fragment.glsl"}
    }));
    shaders.emplace("IZB_structure_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/IZB_structure/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/IZB_structure/fragment.glsl"}
    }));
    shaders.emplace("shadow_volumes_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/shadow_volumes/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/shadow_volumes/fragment.glsl"},
        {GL_GEOMETRY_SHADER, "shaders/shadow_volumes/geometry.glsl"}
    }));
    shaders.emplace("penumbra_wedges_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/penumbra_wedges/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/penumbra_wedges/fragment.glsl"},
        {GL_GEOMETRY_SHADER, "shaders/penumbra_wedges/geometry.glsl"}
    }));
    shaders.emplace("gradient_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/gradient/vertex.glsl"},
        {GL_GEOMETRY_SHADER, "shaders/gradient/geometry.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/gradient/fragment.glsl"}
    }));
    shaders.emplace("IZB_rasterizer_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/IZB_rasterizer/vertex.glsl"},
        {GL_GEOMETRY_SHADER, "shaders/IZB_rasterizer/geometry.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/IZB_rasterizer/fragment.glsl"}
    }));
    shaders.emplace("IZB_shader_drawer_s", new ShaderProgram({
        {GL_VERTEX_SHADER, "shaders/IZB_shader_drawer/vertex.glsl"},
        {GL_FRAGMENT_SHADER, "shaders/IZB_shader_drawer/fragment.glsl"}
    }));
    GL_CHECK_ERRORS;
    objects.emplace("skybox",new SimpleSceneObject(meshes["skybox_mesh"].get()));
    objects.at("skybox")->SetMaterial(PBRMaterial{0.0, 0.5, 0.0, 0.5});
    objects.emplace("plane", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(translate4x4(float3(-3, 0.0, -2)), scale4x4(float3(20, 20, 20)))
    ));

    objects.at("plane")->SetMaterial(PBRMaterial{0.0, 0.99, 0, 0});
    objects.emplace("mirror", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(rotate_X_4x4(-0.0),mul(translate4x4(float3(0, 0.05, 2)), scale4x4(float3(1.0, 1.0, 1.0))))
    ));

    objects.at("mirror")->SetMaterial(PBRMaterial{1, 0.5, 0.95, 0});

    objects.emplace("supercube", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(mul(translate4x4(float3(0.0, 0.56, 0.0)), scale4x4(float3(0.5, 0.5, 2))),
        rotate_Z_4x4(M_PI_2))
    ));
    objects.at("supercube")->SetMaterial(PBRMaterial{0.0,0.0,0.97,0.00});

    obs = new SimpleSceneObject(
        meshes["wall"].get(),
        mul(translate4x4(camera->getPos()), scale4x4(float3(0.25, 0.25, 0.25)))
    );
    objects.emplace("observer",obs );
    objects.at("observer")->SetMaterial(PBRMaterial{0.5,0.5,0.0,0.00});

    objects.emplace("glass", new SimpleSceneObject(
        meshes["plane_mesh"].get(),
        mul(mul(translate4x4(float3(0.0, 2.56, -3.0)), scale4x4(float3(0.5, 1.0, 1))),
        rotate_X_4x4(M_PI_2))
    ));
    
    objects.at("glass")->SetMaterial(PBRMaterial{0.0,0.0,0.0,0.95});
    tank = new ModelSceneObject(models["tank_model"].get());
    //models["tank_model"].get()->GetMeshes()[0]->
    tank->SetTransform(rotate_Y_4x4(-0.0), "Head");
    for (int i=0;i<tank->GetMeshCount();i++)
    {
        tank->show_mesh(i);
        tank->SetTransform(translate4x4(float3(3.0,0.05,0.0)),i);
    }
    objects.emplace("tank", tank);
    objects["tank"]->SetMaterial(PBRMaterial{0.8, 0.4,0.2, 0});

    auto skull = new ModelSceneObject(models["skull_model"].get());
    
    for (int i=0;i<tank->GetMeshCount();i++)
    {
        //skull->show_mesh(i);
        skull->SetTransform(translate4x4(float3(-5.0,0.05,0.0)),i);
    }
    objects.emplace("skull", skull);
    objects["skull"]->SetMaterial(PBRMaterial{0.9, 0.0,0.3, 0.00});
    objects.emplace("screen", new SimpleSceneObject(meshes["screen_mesh"].get()));
    objects.emplace("adj_test2", new SimpleSceneObject(meshes.at("adj_test_mesh2").get(),
    mul(rotate_Y_4x4(M_PI)
    ,mul(scale4x4(float3(1,1,1)),translate4x4(float3(-4.0,0.00,-6.5))))
    ));
    objects.emplace("adj_test_norm2", new SimpleSceneObject(meshes.at("adj_test_mesh_norm2").get(),
    mul(rotate_Y_4x4(M_PI)
    ,mul(scale4x4(float3(1,1,1)),translate4x4(float3(-4.001,0.001,-6.499))))
    ));
    objects["adj_test_norm2"]->SetMaterial(PBRMaterial{0.9, 1,0.0, 0.00});
    
    objects.emplace("adj_test3", new SimpleSceneObject(meshes.at("adj_test_mesh2").get(),
    mul(rotate_Y_4x4(M_PI)
    ,mul(translate4x4(float3(3.95,0.001,-8.5)),scale4x4(float3(0.4,0.4,0.4))))
    ));
    objects.emplace("adj_test_norm3", new SimpleSceneObject(meshes.at("adj_test_mesh_norm2").get(),
    mul(rotate_Y_4x4(M_PI)
    ,mul(translate4x4(float3(3.951,0.001,-8.499)),scale4x4(float3(0.4,0.4,0.4))))
    ));
    objects["adj_test_norm3"]->SetMaterial(PBRMaterial{0.9, 1,0.0, 0.00});

    objects.emplace("adj_test", new SimpleSceneObject(meshes.at("adj_test_mesh").get(),
    mul(rotate_Y_4x4(M_PI)
    ,mul(scale4x4(float3(1,1,1)),translate4x4(float3(0.0,1.00,-6.5))))
    ));
    objects.emplace("adj_test_norm", new SimpleSceneObject(meshes.at("adj_test_mesh_norm").get(),
    mul(rotate_Y_4x4(M_PI)
    ,mul(scale4x4(float3(1,1,1)),translate4x4(float3(0.001,1.001,-6.499))))
    ));
    objects["adj_test_norm"]->SetMaterial(PBRMaterial{0.9, 0.9,0.0, 0.00});
    lightdir = float3(-5.f, 3.f, 3.f);
    light = mul(
    ortho_matr(-10, 10, -10, 10, -15.0, 10.f), 
    transpose4x4(lookAtTransposed(lightdir, float3(0, 0, 0), float3(0, 1, 0))));
    vsm =  new VarianceShadowMap(800, 600);
    csm = new CascadedShadowMap(800,600);
    oc.SetDrawFlags(oc.NORMAL);
    oc.SetShadowsType(oc.SIMPLE);
    point_light = float3(0,2.5,0);
    
}

void Scene::SetRenderQueues()
{
    auto sb = (Skybox*)meshes["skybox_mesh"].get();

    GBufferR.Init(800,600);
    HBAOR.Init(800,600);
    //CSMR.Init(800,600);
    BlurR.Init(800,600,0);
    ScrRefR.Init(800,600);
    LightR.Init(800,600);
    BloomR.Init(800,600);
    CubeMapRefR.Init(200,200, (ISceneObject *)objects.at("supercube").get());
    CubeMapShadowsR.Init(300,300,point_light);
    //SkyBoxR.Init(800,600,sb->GetTexId());
    SkyBoxR.Init(800,600,CubeMapShadowsR.GetCubeMapTex());
    ShVolR.Init(800,600,(ISceneObject *)objects.at("screen").get());
    GradR.Init(800,600);
    IZBufR.Init(800,600,(ISceneObject *)objects.at("screen").get());

    FinalR.SetShader(shaders.at("assemble_s").get());
    FinalR.SetTextures(LightR.GetLightTex(), ScrRefR.GetReflectionsTex(), GBufferR.GetMaterialTex(), 
                        BloomR.GetBloomTex(), HBAOR.GetHBAOTex(), IZBufR.GetShadeTex(), GBufferR.GetColorTex(),
                        ScrRefR.GetRefractionsTex());
    FinalR.AddObject(objects["screen"].get());

    LightR.SetShader(shaders["pbr_lighting_s"].get());
    LightR.SetTextures(GBufferR.GetColorTex(), GBufferR.GetNormalTex(), GBufferR.GetVertexTex(), GBufferR.GetMaterialTex());
    LightR.SetLightSrc(0, float3(-50.f, 30.f, -5.f), float3(40000.f, 40000.f, 40000.f));
    LightR.SetLightSrc(1, float3(5.f, 2.f, 0.f), float3(0.f, 0.f, 00.f));
    LightR.SetLightSrc(2, float3(0.f, 2.f, 5.f), float3(0.f, 00.f, 00.f));
    LightR.SetLightSrc(3, float3(0.f, -5.f, -5.f), float3(0.f, 0.f, 0.f));
    LightR.AddObject(objects["screen"].get());

    ScrRefR.SetShader(shaders.at("reflect_s").get());
    ScrRefR.SetTextures(LightR.GetLightTex(), GBufferR.GetNormalTex(), GBufferR.GetDepthTex(), 
          GBufferR.GetVertexTex(), GBufferR.GetMaterialTex(),CubeMapRefR.GetCubeMapTex(), sb->GetTexId());
    ScrRefR.SetProj(projection); 
    ScrRefR.AddObject(objects["screen"].get());

    GBufferR.SetShader(shaders["gbuffer_s"].get());
    GBufferR.SetProj(projection);
    GBufferR.SetShadowsTextures(vsm->GetDepthTex(),vsm->GetTexture(),csm->GetCascade(0),csm->GetCascade(1),
                                csm->GetCascade(2),CubeMapShadowsR.GetCubeMapTex());
    GBufferR.AddObject(objects["plane"].get());
    GBufferR.AddObject(objects["tank"].get());
    //GBufferR.AddObject(objects.at("mirror").get());
    //GBufferR.AddObject(objects["skull"].get());
    //GBufferR.AddObject(objects.at("supercube").get());
    //GBufferR.AddObject(objects.at("glass").get());
    GBufferR.AddObject(objects.at("adj_test_norm").get());
    GBufferR.AddObject(objects.at("adj_test_norm2").get());
    GBufferR.AddObject(objects.at("adj_test_norm3").get());

    BlurR.SetShader(shaders["blur_s"].get());
    BlurR.AddObject(objects["screen"].get());


    QuadR.SetShader(shaders["texture_draw_s"].get());
    //QuadR.SetTexture(ShVolR.GetShadeTex());
    QuadR.SetTexture(IZBufR.GetShadeTex());
    //QuadR.SetTexture(HBAOR.GetHBAOTex());
    QuadR.AddObject(objects["screen"].get());


    /*CSMR.SetShader(shaders["csm_s"].get());
    CSMR.SetLight(lookAtTransposed(lightdir, float3(0, 0, 0), float3(0, 1, 0)));
    CSMR.AddObject(objects["plane"].get());
    CSMR.AddObject(objects["tank"].get());*/

    HBAOR.SetShader(shaders["hbao_s"].get());
    HBAOR.SetTextures(GBufferR.GetVertexTex(),GBufferR.GetNoiseTex(), GBufferR.GetMaterialTex());
    HBAOR.AddObject(objects["screen"].get());
    GL_CHECK_ERRORS;

    BloomR.SetShader(shaders["bloom_s"].get());
    BloomR.SetTexture(LightR.GetLightTex());
    BloomR.AddObject(objects["screen"].get());
    GL_CHECK_ERRORS;

    VSMRQ.SetShaderProg(shaders["vsm_s"].get());
    VSMRQ.setVSM(vsm);
    VSMRQ.SetLight(light);
    VSMRQ.AddSceneObject(objects["plane"].get());
    VSMRQ.AddSceneObject(objects["tank"].get());
    VSMRQ.AddSceneObject(objects["skull"].get());
    VSMRQ.AddSceneObject(objects.at("supercube").get());
    VSMRQ.AddSceneObject(objects.at("glass").get());

    CSMRQ.SetShaderProg(shaders["vsm_s"].get());
    CSMRQ.SetLight(lookAtTransposed(lightdir, float3(0, 0, 0), float3(0, 1, 0)));
    //CSMRQ.SetLight(transpose4x4(cam));
    CSMRQ.setCSM(csm);
    CSMRQ.AddSceneObject(objects["plane"].get());
    CSMRQ.AddSceneObject(objects["tank"].get());
    CSMRQ.AddSceneObject(objects["skull"].get());
    CSMRQ.AddSceneObject(objects.at("supercube").get());
    CSMRQ.AddSceneObject(objects.at("glass").get());
    
    SkyBoxR.SetShader(shaders["skybox_s"].get());
    SkyBoxR.AddObject(objects["skybox"].get());

    CubeMapRefR.SetShader(shaders["tank_s"].get());
    CubeMapRefR.AddObject(objects["plane"].get());
    CubeMapRefR.AddObject(objects["tank"].get());
    CubeMapRefR.AddObject(objects["skull"].get());
    CubeMapRefR.AddObject(objects["observer"].get());

    CubeMapShadowsR.SetShader(shaders["simple_depth_s"].get());
    CubeMapShadowsR.AddObject(objects["plane"].get());
    CubeMapShadowsR.AddObject(objects["tank"].get());
    CubeMapShadowsR.AddObject(objects["skull"].get());

    //ShVolR.SetShadeTex(GBufferR.GetShadedTex());
    ShVolR.SetShaders(shaders["depth_draw_s"].get(),
                      shaders["shadow_volumes_s"].get(),
                      shaders["penumbra_wedges_s"].get(),
                      shaders["stencil_test_s"].get(),
                      shaders["penumbra_drawer_s"].get());
    ShVolR.AddObject(objects["plane"].get());
    ShVolR.AddObject(objects["tank"].get());
    ShVolR.AddObject(objects["skull"].get());
    ShVolR.AddObject(objects.at("adj_test_norm").get());
    ShVolR.AddObject(objects.at("adj_test_norm2").get());
    ShVolR.AddObject(objects.at("adj_test_norm3").get());
    ShVolR.AddOccluder(objects.at("adj_test").get());
    ShVolR.AddOccluder(objects.at("adj_test2").get());
    ShVolR.AddOccluder(objects.at("adj_test3").get());

    GradR.SetShader(shaders["gradient_s"].get());
    GradR.SetTexture(ShVolR.GetShadeTex(),GBufferR.GetVertexTex(),GBufferR.GetNormalTex());
    GradR.AddObject(objects["screen"].get());

    IZBufR.SetTextures(GBufferR.GetVertexTex(),GBufferR.GetDepthTex());
    IZBufR.SetShaders(shaders["IZB_structure_s"].get(),
                      shaders["IZB_rasterizer_s"].get(),
                      shaders["IZB_shader_drawer_s"].get());
    //IZBufR.AddObject(objects["screen"].get());
    IZBufR.AddObject(objects["plane"].get());
    IZBufR.AddObject(objects["tank"].get());
    IZBufR.AddObject(objects["adj_test_norm"].get());
    IZBufR.AddObject(objects["adj_test_norm2"].get());
    IZBufR.AddObject(objects["adj_test_norm3"].get());
    GL_CHECK_ERRORS;
}

void Scene::Render()
{
    obs->SetTransform(mul(translate4x4(camera->getPos()), scale4x4(float3(0.25, 0.25, 0.25))));
    static long tick=0;    
    static long double average_fps = 0.0;
    static clock_t time=clock();
    clock_t delay = clock();
    delay = delay - time;
    time +=delay;
    fprintf(stderr,"<delay = %d>",(int)delay);
    float4x4 cam = camera->getViewMatrix();
    float4x4 caminv = inverse4x4(cam);
    if ((tick%SHADOWS_RENDER_DELAY==0)||(oc.GetDrawFlags()&oc.SHADOWS))
    {
        if (oc.GetShadowsType()==oc.CASCADED)
        {
            CSMRQ.SetCurCamera(transpose4x4(cam));
            csm->BindTarget();
            CSMRQ.OptimalDraw(tick/SHADOWS_RENDER_DELAY);
            csm->Unbind(800,600);
            BlurR.Blur(csm->GetCascade(0),0);
            if ((tick/SHADOWS_RENDER_DELAY)%3==0) BlurR.Blur(csm->GetCascade(1),0);
            if ((tick/SHADOWS_RENDER_DELAY)%9==0) BlurR.Blur(csm->GetCascade(2),0);
        }
        if ((oc.GetShadowsType()==oc.VSM)||
            (oc.GetShadowsType()==oc.SIMPLE))
        {
            vsm->BindTarget();
            VSMRQ.Draw();
            vsm->Unbind(800,600);
            //if (oc.GetShadowsType()==oc.VSM) BlurR.Blur(vsm->GetTexture(),1);
            BlurR.Blur(vsm->GetTexture(),1);
        }
        if (oc.GetShadowsType()==oc.CUBEMAP)
        {
            CubeMapShadowsR.Render();
        }
    }
    GBufferR.AttachBuffer();
    GBufferR.SetBufferRenderState();
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
    GBufferR.ClearBuffer();
    GBufferR.SetView(cam);
    GBufferR.SetLights(light,CSMRQ.GetLights(),CSMRQ.GetCasZBorders(projection),point_light);
    GBufferR.SetShadows(oc.GetShadowsType());
    GBufferR.Render();
    glDepthFunc(GL_LEQUAL);
    SkyBoxR.Render(projection,cam);
    glDepthFunc(GL_LESS);
    BlurR.Blur(GBufferR.GetShadedTex(),2);
    GBufferR.UnattachBuffer();
    //BlurR.Blur(GBufferR.GetMaterialTex(),0);
    LightR.Attach();
    glClearColor(0.f, 0.0f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    LightR.SetView(cam);
    LightR.Render();
    LightR.Unattach(800,600);

    CubeMapRefR.Render();
    
    ScrRefR.Attach();
    ScrRefR.Render(camera->getPos(),cam);
    BlurR.Blur(ScrRefR.GetReflectionsTex(),0);
    ScrRefR.Unattach(800,600);

    //HBAOR.Render(M_PI/2+0.005,0.1,100);
    //BlurR.Blur(HBAOR.GetHBAOTex(),0);
    
    BloomR.Render();
    BlurR.Blur(BloomR.GetBloomTex(),1);

    //ShVolR.Render(cam,projection,camera->getPos());
    //GradR.Render(cam,projection,caminv);

    IZBufR.Render(cam,projection,caminv,light);

    glClearColor(0.f, 1.0f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    FinalR.Render(&oc);
    //QuadR.Render();
    
    tick++;
    int udelay =(int)( delay*1000000/CLOCKS_PER_SEC);
    average_fps +=1000000.0/udelay;
    if (udelay<1000000.0/60.0)
    {
        //usleep((__useconds_t)(udelay-1000000.0/60.0));
    }
    if (tick%100==0)
    {
        fprintf(stderr,"%.1f\n",(float)(average_fps/tick));
        tick = 0;
        average_fps = 0;
    }
    static float phi=0.0;
    phi+=0.05;
    camera->rotateYPR(float3(0.05,0,0));
    camera->setPos(float3(3+7*sin(phi),2.2,3+7*cos(phi)));
}
Scene::~Scene()
{
    delete(vsm);
    delete(csm);
}
float3 Scene::MoveTank(float HeadAngle,float BodyAngle, float movement)
{
    static float HeadRotation = 0.0;
    static float BodyRotation = 0.0;
    static float3 Position = float3(0.0,0.0,0.0);
    HeadRotation+=HeadAngle;
    if (HeadRotation>M_PI) HeadRotation-=2*M_PI;
    else if (HeadRotation<-M_PI) HeadRotation+=2*M_PI; 

    BodyRotation+=BodyAngle;
    if (BodyRotation>M_PI) BodyRotation-=2*M_PI;
    else if (BodyRotation<-M_PI) BodyRotation+=2*M_PI; 
    
    Position +=float3(movement* std::cos(BodyRotation),0.0,-movement* std::sin(BodyRotation));
    tank->SetTransform(mul(translate4x4(Position),rotate_Y_4x4(HeadRotation)), "Head");
    tank->SetTransform(mul(translate4x4(Position),rotate_Y_4x4(BodyRotation)), "Body");
    return(Position);
}