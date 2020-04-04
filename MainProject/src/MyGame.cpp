#include "MyGame.h"
#include <memory>
#include <algorithm>
#include <cmath>
#include <glad/glad.h>
#include <iostream>
#include <random>
#include <vector>

#include "Scenev2.hpp"
#include "RenderPass.hpp"

//Roman's shit
void Show(float4x4 matr) {
    for (int i = 0; i < 4; i++)
    {
        std::cout<<matr.row[i].x<<" ";
        std::cout<<matr.row[i].y<<" ";
        std::cout<<matr.row[i].z<<" ";
        std::cout<<matr.row[i].w<<" ";
        std::cout<<std::endl;
    }
      
}
// Main Game Logic and so on
std::default_random_engine generator;
float4x4 ortho(float left, float right, float bot, float top, float near, float far)
{
  float4x4 result;
  result.set_row(0, float4(2/(right - left), 0.f, 0.f, -(right + left)/(right - left)));
  result.set_row(1, float4(0.f, 2/(top - bot), 0.f, -(top+bot)/(top - bot)));
  result.set_row(2, float4(0.f, 0.f, -2/(far - near), -(far + near)/(far-near)));
  result.set_row(3, float4(0.f, 0.f, 0.f, 1.f));
  return result;
}

MyGame::MyGame()
{

  gw = new GameWindow(800, 600, "CG/2 Task 2. 2019");
  fc = new FreeCamera();
  tc = new TankCamera();
  fc->setPos(float3(0.966, 2.11, -0.116));
  tc->setPos(float3(-5,1.05,-9));
  my_cam = fc;
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  FOVAngles = 90;
  MaxDistance = 500;
  MinDistance = 0.01;
  ratio = double(gw->getViewWidth())/gw->getViewHeight();
  projection = transpose(projectionMatrixTransposed(FOVAngles,ratio,MinDistance,MaxDistance));
  
  if(!mainScene.LoadScene("assets/New_Models/new_spoonza.gltf")){
    throw(std::runtime_error("Scene load failed"));
  }
  mainScene.SetCamera(projection, my_cam->getViewMatrix(), FOVAngles * M_PI/180.f, MinDistance, MaxDistance);
  
  bool status = mainScene.LoadEnvMap("assets/ambient.hdr");
  if(!status){
    throw(std::runtime_error("EnvMap load failed"));
  }

  auto mt = new GLTFModel("assets/Tank_M60A1_opt/tank_m60a1.gltf");
  mainScene.tank = new ModelSceneObject(mt);
  for (int i=0;i<mainScene.tank->GetMeshCount();i++)
  {
    mainScene.tank->SetTransform(mul(translate4x4(float3(-5,0.75,-9)),scale4x4(float3(0.3,0.3,0.3))),i);
  }
  
  std::vector<std::string> faces {
    "assets/ame_starfield/starfield_bk.tga",
    "assets/ame_starfield/starfield_dn.tga",
    "assets/ame_starfield/starfield_ft.tga",
    "assets/ame_starfield/starfield_lf.tga",
    "assets/ame_starfield/starfield_rt.tga",
    "assets/ame_starfield/starfield_up.tga"
  };
  if(!mainScene.LoadSkybox(faces)){
    throw(std::runtime_error("Skybox load failed"));
  }

  const float3 fireColor = float3(0.8, 0.45, 0.1);

  GL_CHECK_ERRORS;
  renderer::SimplePointLight l;
  l.position = float3(-0.253391, 0.81, -1.48683);
  l.color = fireColor;
  mainScene.lights.simplePointLights.Set(0, l);

  renderer::SimplePointLight l2;
  l2.position = float3(0.990104, 0.81, -0.374768);
  l2.color = fireColor;
  mainScene.lights.simplePointLights.Set(1, l2);

  renderer::SimplePointLight l3;
  l3.position = float3(0.615916, 0.81, 0.0206439);
  l3.color = fireColor;
  mainScene.lights.simplePointLights.Set(2, l3);

  renderer::SimplePointLight l4;
  l4.position = float3(-0.593332, 0.81, -1.06741);
  l4.color = fireColor;
  mainScene.lights.simplePointLights.Set(3, l4);

  renderer::SimplePointLight l5;
  l5.position = float3(-1.7, 1.1, -1.45);
  l5.color = float3(0, 1, 0);
  mainScene.lights.simplePointLights.Set(3, l5);

  mainScene.lights.simplePointLights.SetLightsCount(4);

  renderer::ShadowMapPointLight spl{float3(500, 100, 100), float3(-2.9, 4.8, 2.2), 
    float3(-4.5, 3.0, 3.0), float3(0, 1, 0), 0.1, 20, 1500, 1500, M_PI_2};
  mainScene.lights.SMPointLights.Set(0, spl);

  renderer::ShadowMapPointLight spl2{
    float3(0.8, 0.8, 0.8), float3(1.38529, 0.786953, 0.704671), float3(1.20701, 0.76638, 0.805966), float3(0, 1, 0), 
    0.05, 5, 512, 512, M_PI_2
  };
  mainScene.lights.SMPointLights.Set(1, spl2);
  mainScene.lights.SMPointLights.SetLightsCount(2);
  
  renderer::VSMLight vsm{
    float3(400, 400, 400),
    float3(-3, 6, -7), float3(-4, 5, -7), float3(0, 1, 0),
    1, 70, 1024, 1024, M_PI_2, 3
  };
  mainScene.lights.VSMLights.Set(0, vsm);
  

  renderer::VSMLight vsm2{
    float3(100, 100, 500),
    float3(-0.151492, 3.48077, 10.1466), float3(-1.4019, 2.78474, 10.6355), float3(0, 1, 0),
    1, 30, 1024, 1024, M_PI_2, 3
  };
  mainScene.lights.VSMLights.Set(1, vsm2);
  mainScene.lights.VSMLights.SetLightsCount(2);

  renderer::CMPointLight cpl{ 
    float3(1, 1, 1), float3(0.375,2.1, -0.745), 1024, 0.1, 20};
  mainScene.lights.CMLights.Set(0, cpl);
  mainScene.lights.CMLights.SetLightsCount(1);

  renderer::SVPointLight svl{renderer::SVPointLight::POINT,false, float3(15,5,5),
    float3(3.976,1.766,-1.6734),0.05,4.0};//float3(26.3,1.2,14.4)float3(27.66,1.2,16.29)
  mainScene.lights.SVLights.Set(0, svl);
  renderer::SVPointLight svl2{renderer::SVPointLight::SPHERE,false,float3(5,5,15),
    float3(3.3286,2.111,-2.098),0.05,10.0};//float3(4.0,1.5,0.3)
    svl2.lps = new ParticleSystem();
    svl2.lps->Init(100,float3(2.8,1.8,-2.68),0.03,float3(0,0,1),true,svl2.lps->JOINT);
  mainScene.lights.SVLights.Set(1, svl2);
  mainScene.lights.SVLights.SetLightsCount(2);
  
  renderer::RSMLight rsml;
  rsml.color = float3(0, 2, 0);
  rsml.cosConeAngle = cos(M_PI_2*0.8);
  rsml.mapHeight = rsml.mapWidth = 512;
  rsml.normConeDir = normalize(float3(0, -0.2, 1));
  rsml.position = float3(-1.7, 1.1, -1.45);
  rsml.view = transpose4x4(lookAtTransposed(rsml.position, float3(-1.31, 0.2, -1.34), float3(0, 1, 0)));
  rsml.projection = transpose4x4(projectionMatrixTransposed(90*0.8, 1, 0.01, 2));
  
  mainScene.lights.RSMLights.Set(0, rsml);
  mainScene.lights.RSMLights.SetLightsCount(1);

  
  renderer::Config conf;
  conf.useReflections = true;
  conf.useAO = true;
  conf.useBloom = true;
  conf.useShadows = true;
  conf.useIndirectLighting = true;
  conf.useIBL = true;
  renderer.SetConf(conf);
  renderer.Init(&mainScene, int4(0, 0, gw->getViewWidth(), gw->getViewHeight()));
  
  
  
  /*
  scene.SetViewProj(projection, fc);
  scene.LoadData();
  scene.CreateObjects();
  
  renderer::Scene scene;
  if(scene.LoadScene("assets/Tank_M60A1_opt/tank_m60a1.gltf"))
  {
    for(const auto& mesh : scene.meshes){
      std::cout << mesh.name << " " << mesh.primitives.size() << "\n";
      for(const auto &prim : mesh.primitives){
        std::cout << "prim : " << prim.materialId << " " << prim.hasIndexies << "\n"; 
      }
    }
    for(const auto & node : scene.nodes){
      std::cout << "node name : " << node.name << "\n";
    }
    for(const auto &mat : scene.materials){
      std::cout << "material " << mat.name << " " << mat.albedoTex << " " << mat.normalTex << "\n";
    }
  }*/
}

void MyGame::createShadows()
{
  vshadowmap->BindTarget();
  glClearColor(0.f, 0.f, 0.f, 0.f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  vsm_s->StartUseShader();
  for(auto mesh : my_tank->GetMeshes())
  {
      float4x4 model;
      if(mesh->GetName() == "Head"){
        auto angle = tc->getYPR();
        model = mul(rotate_Y_4x4(angle.x), mesh->GetTransform());
      } else {
        model = mesh->GetTransform();
      }
      vsm_s->SetUniform("MVP", mul(light, model));
      mesh->Draw();
  }
  
  vsm_s->SetUniform("MVP", mul(light, scale4x4(float3(100.f, 100.f, 100.f))));
  plane_gr->Draw();

  for(auto& mat : box_model){
    vsm_s->SetUniform("MVP", mul(light, mat));
    box_targ->Draw();
  }

  vsm_s->StopUseShader();
  vshadowmap->Unbind(800, 600);
  GL_CHECK_ERRORS;
}

MyGame::~MyGame()
{
  // Free all resources
  delete gw;
  delete fc;
  delete tc;
  delete simp_s;
  delete grid_s;
  delete targ_s;
  delete my_stealth_tank;
  delete tank_s;
  delete shadowmap_s;
  delete depthbuff_s;
  delete my_tank;
  delete box_targ;
  delete plane_gr;
  delete triangle;
  delete model;
  delete shadowmap;
  delete vshadowmap;
  delete vsm_buff_draw_s;
  delete vsm_s;
  delete filter_fb;
  delete SSAO_s;
  delete SSAO_visualizer_s;
}

void MyGame::start()
{
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  while (!glfwWindowShouldClose(gw->get()))
  {
    gw->update();
    update(gw->getState());

    draw(gw->getState());
    glfwSwapBuffers(gw->get());
  }
}

bool key_released(const GameState &gs, int key)
{
  // key not pressed for now but was pressed in prev frame
  return !gs.keyboard[key] && gs.prev.keyboard[key];
}
bool key_pressed(const GameState &gs, int key)
{
  return gs.keyboard[key];
}
bool mbtn_released(const GameState &gs, int key)
{
  return !gs.mouse[key] && gs.prev.mouse[key];
}

void MyGame::update(const GameState &gs)
{
  if (key_released(gs, GLFW_KEY_ESCAPE))
    glfwSetWindowShouldClose(gw->get(), GL_TRUE);

  if (key_released(gs, GLFW_KEY_F))
    my_cam = (my_cam == fc) ? (Camera*)tc : (Camera*)fc;

  if (key_released(gs, GLFW_KEY_SPACE))
  {
    float3 pos = my_cam->getPos();
    std::cout << "Camera pos: " << pos.x << ' ' << pos.y << ' ' << pos.z << std::endl;
  }
  if (my_cam==fc)
  {
      if(key_released(gs, GLFW_KEY_P))
      {
        renderer.config.tex_switch = !renderer.config.tex_switch;
      }
      if(key_released(gs, GLFW_KEY_1))
      {
        if (renderer.config.tex_switch) renderer.texDraw.SetState(1);
        else fc->setPos(float3(-1.12,0.87,-0.87));
      }
      if(key_released(gs, GLFW_KEY_2))
      {
        if (renderer.config.tex_switch) renderer.texDraw.SetState(2);
        else fc->setPos(float3(-4.6,3,-8.66));
      }
      if(key_released(gs, GLFW_KEY_3))
      {
        if (renderer.config.tex_switch) renderer.texDraw.SetState(3);
        else fc->setPos(float3(-9.4,2.6,4.7));
      }
      if(key_released(gs, GLFW_KEY_4))
      {
        if (renderer.config.tex_switch) renderer.texDraw.SetState(4);
        else fc->setPos(float3(0.34,1,-0.57));
      }
      if(key_released(gs, GLFW_KEY_5))
      {
        if (renderer.config.tex_switch) renderer.texDraw.SetState(5);
        else fc->setPos(float3(-3,1.6,9.7));
      }
      if(key_released(gs, GLFW_KEY_6))
      {
        if (renderer.config.tex_switch) renderer.texDraw.SetState(6);
        else fc->setPos(float3(0.64,0.8,-0.196));
      }
      if(key_released(gs, GLFW_KEY_7))
      {
        if (renderer.config.tex_switch) renderer.texDraw.SetState(7);
        else fc->setPos(float3(3.09,0.91,-1.97));
      }
      if(key_released(gs, GLFW_KEY_8))
      {
        if (renderer.config.tex_switch) renderer.texDraw.SetState(8);
        else fc->setPos(float3(1.06,0.8,0.41));
      }
      if(key_released(gs, GLFW_KEY_9))
      {
        if (renderer.config.tex_switch) renderer.texDraw.SetState(9);
      }
  }
  else
  {
    if(key_pressed(gs, GLFW_KEY_R))
      {
        MoveTank(0.01,0,0);
        tc->rotateYPR(float3(0.01,0.0,0.0));  
      }
      if(key_pressed(gs, GLFW_KEY_T))
      {
        MoveTank(-0.01,0,0);
        tc->rotateYPR(float3(-0.01,0.0,0.0));
      }
      if(key_pressed(gs, GLFW_KEY_W))
      {
        float3 pos = MoveTank(0,0,0.02)+float3(0.0,0.3,0.0);
        tc->setPos(pos);
        //fprintf(stderr,"%f %f %f\n",pos.x,pos.y,pos.z);
      }
      if(key_pressed(gs, GLFW_KEY_S))
      {
        float3 pos = MoveTank(0,0,-0.02);
        tc->setPos(pos+float3(0.0,0.3,0.0));
      }
      if(key_pressed(gs, GLFW_KEY_A))
      {
        float3 pos = MoveTank(0,0.01,0.0);
        tc->setPos(pos+float3(0.0,0.3,0.0));
      }
      if(key_pressed(gs, GLFW_KEY_D))
      {
        float3 pos = MoveTank(0,-0.01,0.0);
        tc->setPos(pos+float3(0.0,0.3,0.0));
      }
  }
  
  if (mbtn_released(gs, GLFW_MOUSE_BUTTON_RIGHT))
    glfwSetInputMode(gw->get(), GLFW_CURSOR,
                     gs.cursorCaptured ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

  my_cam->update(gs);
}

void MyGame::setGSigma(float s)
{
  float weight[5] {0.f, 0.f, 0.f, 0.f, 0.f};
  for(int i = 0; i < 5; i++)
  {
    weight[i] = 1.f/(2.f * M_PI * s *s) * std::exp(-(0.6 * i/(2.f * s * s)));
    std::cout << weight[i] << "\n";
  }
  gaucean_s->StartUseShader();
  auto prog = gaucean_s->GetProgram();
  auto loc = glGetUniformLocation(prog, "weight");
  if(loc == -1){
    GL_CHECK_ERRORS;
    throw std::logic_error("No weights found");
  }
  glUniform1fv(loc, 5, weight);
  gaucean_s->StopUseShader();
}

void MyGame::createBlur()
{
  GLuint tempTex;
  glGenTextures(1, &tempTex);
  glBindTexture(GL_TEXTURE_2D, tempTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 1024, 1024, 0, GL_RG, GL_FLOAT, (void*)0);
        
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  filter_fb->Bind();
  auto framebuff = filter_fb->Get();
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tempTex, 0);
  glBindTexture(GL_TEXTURE_2D, vshadowmap->GetTexture());
  glViewport(0, 0, 1024, 1024);
  gaucean_s->StartUseShader();
  gaucean_s->SetUniform("isHorizontal", 1);
  screen->Draw();
  gaucean_s->SetUniform("isHorizontal", 0);
  glBindTexture(GL_TEXTURE_2D, tempTex);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, vshadowmap->GetTexture(), 0);
  screen->Draw();
  gaucean_s->StopUseShader();
  filter_fb->Unbind();
  glDeleteTextures(1, &tempTex);
  glViewport(0, 0, 800, 600);
}
void MyGame::createBuffer()
{
    texMap->BindTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ShaderZone drawZone {this, default_s};
    default_s->SetUniform("use_diffuse_tex", 1);
    for(auto mesh : my_tank->GetMeshes()){
      float4x4 model;
      model.identity();
      if(mesh->GetName() == "Head"){
        auto angle = tc->getYPR();
        model = mul(model, mul(rotate_Y_4x4(angle.x), mesh->GetTransform()));
      } else {
        model = mul(model, mesh->GetTransform());
      }
      default_s->SetUniform("model", model);
      mesh->Draw(); 
    }
    default_s->SetUniform("use_diffuse_tex", 0);
    default_s->SetUniform("model", scale4x4(float3(100.f, 100.f, 100.f)));
    default_s->SetUniform("default_color", float3(0.f, 0.66f, 0.0f));
    //plane_gr->Draw();
    default_s->SetUniform("default_color", float3(0.0f, 0.66f, 0.0f));
    for(auto& mat : box_model){
      default_s->SetUniform("model", mat);
      //box_targ->Draw();
    }
    texMap->Unbind(800, 600);
  }
#define SHADER_ZONE(shader) ShaderZone ShaderZoneGuardName ## __LINE__(this, shader);

void MyGame::calculate_SSAO()
{ 
  int s_count=64;
  std::uniform_real_distribution<float> randomFloats(0.0, 1.0); 
  
  std::vector<float3> ssaoKernel;
  float scale;
  for (unsigned int i = 0; i < s_count; ++i)
  {
      float3 sample(
          randomFloats(generator) * 2.0 - 1.0, 
          randomFloats(generator) * 2.0 - 1.0, 
          randomFloats(generator)
      );
      sample  = normalize(sample);
      sample *= randomFloats(generator);
      float scale = (float)i / (s_count+0.0);  
  scale   = lerp(0.1f, 1.0f, scale * scale);
  sample *= scale;
  ssaoKernel.push_back(sample);  
  }

  std::vector<float3> ssaoNoise;
  for (unsigned int i = 0; i < 16; i++)
  {
      float3 noise(
          randomFloats(generator) * 2.0 - 1.0, 
          randomFloats(generator) * 2.0 - 1.0, 
          0.0f); 
      ssaoNoise.push_back(noise);
  } 
  
  GLfloat xs[s_count],ys[s_count],zs[s_count];
  const GLfloat *xp=xs,*yp=ys,*zp=zs;
  for (int i=0;i<s_count;i++)
  {
    xs[i]=ssaoKernel[i].x;
    ys[i]=ssaoKernel[i].y;
    zs[i]=ssaoKernel[i].z;
    
  }
  postEffectsMap->BindTarget();
  glClearColor(1.0f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  SSAO_s->StartUseShader();
  auto prog = SSAO_s->GetProgram();
  auto loc = glGetUniformLocation(prog, "samplesx");
  glUniform1fv(loc,64,xp);
  loc = glGetUniformLocation(prog, "samplesy");
  glUniform1fv(loc,64,yp);
  loc = glGetUniformLocation(prog, "samplesz");
  glUniform1fv(loc,64,zp);  

  SSAO_s->SetUniform("projection",my_cam->getViewMatrix());
  SSAO_s->SetUniform("diftex", 1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D,texMap->GetTexture());

  SSAO_s->SetUniform("normtex", 2);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, texMap->GetNormTex());

  SSAO_s->SetUniform("depthtex", 3);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, texMap->GetDepthTex());

  screen->Draw();
  SSAO_s->StopUseShader();
  postEffectsMap->Unbind(800,600);
}
void MyGame::draw(const GameState &gs)
{ 
  //scene.Render();  
  mainScene.sceneCamera.position = my_cam->getPos();
  mainScene.SetCamera(my_cam->getViewMatrix());
  renderer.Draw();
}

// Shader Zone helper
MyGame::ShaderZone::ShaderZone(MyGame *g, Shader *shader) : shader(shader), parent(g)
{
  shader->StartUseShader();
  shader->SetUniform("projection", parent->projection);
  shader->SetUniform("view", parent->my_cam->getViewMatrix());
  shader->SetUniform("lightNDC", parent->light);
  shader->SetUniform("use_shadow_tex", 1);
  shader->SetUniform("vsm_tex", 11);
  glActiveTexture(GL_TEXTURE11);
  glBindTexture(GL_TEXTURE_2D, parent->vshadowmap->GetTexture());
  glActiveTexture(GL_TEXTURE0);
}

MyGame::ShaderZone::~ShaderZone()
{
  shader->StopUseShader();
}
float3 MyGame::MoveTank(float HeadAngle,float BodyAngle, float movement)
{
    static float HeadRotation = 0.0;
    static float BodyRotation = 0.0;
    static float3 Position = float3(-5,0.75,-9);
    static float4x4 scale = scale4x4(float3(0.3,0.3,0.3));
    HeadRotation+=HeadAngle;
    if (HeadRotation>M_PI) HeadRotation-=2*M_PI;
    else if (HeadRotation<-M_PI) HeadRotation+=2*M_PI; 

    BodyRotation+=BodyAngle;
    if (BodyRotation>M_PI) BodyRotation-=2*M_PI;
    else if (BodyRotation<-M_PI) BodyRotation+=2*M_PI; 
    
    Position +=float3(movement* std::cos(BodyRotation),0.0,-movement* std::sin(BodyRotation));
    mainScene.tank->SetTransform(mul(mul(translate4x4(Position),rotate_Y_4x4(HeadRotation)),scale), "Head");
    mainScene.tank->SetTransform(mul(mul(translate4x4(Position),rotate_Y_4x4(BodyRotation)),scale), "Body");
    return(Position);
}

// Singleton
std::unique_ptr<MyGame> my_game;
void init_my_game(){ my_game = std::make_unique<MyGame>(); }
void close_my_game(){ my_game.reset(); }
MyGame *get_my_game(){ return my_game.get(); }
