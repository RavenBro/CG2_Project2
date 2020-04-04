#include "MyGame.h"
#include <memory>
#include <algorithm>
#include <cmath>
#include "framework/LoaderGLTF.h"
#include <glad/glad.h>
#include <iostream>
#include <random>
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
  fc->setPos(float3(0, 0.3, 5));
  my_cam = fc;
  triangle = CreateSimpleTriangleMesh();
  plane_gr = CreateSimplePlaneMesh();
  box_targ = CreateSimpleBoxMesh();
 
  screen = CreateQuad();
  filter_fb = new FrameBuffer();
  tank_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/tank/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/tank/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  grid_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/grid/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/grid/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  depthbuff_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/depthbuff/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/depthbuff/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  shadowmap_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/shadowmap/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/shadowmap/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  targ_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/targets/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/targets/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  default_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/default/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/default/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  vsm_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/vsm/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/vsm/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  vsm_buff_draw_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/vsm_buff_draw/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/vsm_buff_draw/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  gaucean_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/gaucean_blur/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/gaucean_blur/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  SSAO_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/SSAO/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/SSAO/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  SSAO_visualizer_s = new ShaderProgram({
    {GL_VERTEX_SHADER, "shaders/SSAO_visualizer/vertex.glsl"},
    {GL_FRAGMENT_SHADER, "shaders/SSAO_visualizer/fragment.glsl"}
  }); GL_CHECK_ERRORS;

  my_tank = new GLTFModel("assets/Tank_M60A1_opt/tank_m60a1.gltf");
  for(auto mesh : my_tank->GetMeshes())
  {
    if(mesh->GetName() == "Head")
      tc->setPos(to_float3(mesh->GetTransform().get_col(3)));
  }

  projection = transpose4x4(projectionMatrixTransposed(90, 4.f/3, 0.1, 100));//A20.11
  shadowmap = new ShadowMap(1024, 1024);
  vshadowmap = new VarianceShadowMap(1024, 1024);
  light = mul(
    ortho(-10, 10, -10, 10, 0, 20.f), 
    transpose4x4(lookAtTransposed(float3(0.f, 5.f, 7.f), float3(0, 0, 0), float3(0, 1, 0))));

  box_model.push_back(mul(translate4x4(float3(0-3.f, 1.f, 1.5f)), scale4x4(float3(1.f, 3.f, 1.f))));
  box_model.push_back(mul(translate4x4(float3(5.f, 2.f, 0.f)), rotate_Y_4x4(0.8)));
  setGSigma(0.738f);
  texMap.reset(new TextureMap(800, 600));
  postEffectsMap.reset(new PostEffectsMap(800,600));
  GL_CHECK_ERRORS;
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
  glEnable(GL_DEPTH_TEST);
  //createShadows(); 
  // MainLoop
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

  if(key_released(gs, GLFW_KEY_2))
  {
    drawDepthBuff = !drawDepthBuff;  
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
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
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
  glEnable(GL_DEPTH_TEST);
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
  /*std::cout<<"(";
  std::cout<<xs[1];
  std::cout<<" ";
  std::cout<<ys[1];
  std::cout<<" ";
  std::cout<<zs[1];
  std::cout<<")";*/
    auto loc = glGetUniformLocation(prog, "samplesx");
  glUniform1fv(loc,64,xp);
  loc = glGetUniformLocation(prog, "samplesy");
  glUniform1fv(loc,64,yp);
  loc = glGetUniformLocation(prog, "samplesz");
  glUniform1fv(loc,64,zp);  

  SSAO_s->SetUniform("projection", projection);
  SSAO_s->SetUniform("diftex", 1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D,texMap->GetPosTex());//A20.11

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
  
  
  createBuffer();
  createShadows();
  createBlur();
  calculate_SSAO();
  if(true)
  { 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SSAO_s->StartUseShader();
    glBindTexture(GL_TEXTURE_2D, postEffectsMap->GetSSAOTex());
    screen->Draw();
    SSAO_s->StopUseShader();
  } 
  else
  {
    glClearColor(0.1f, 0.6f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    vsm_s->StartUseShader();
    glBindTexture(GL_TEXTURE_2D, vshadowmap->GetTexture());
    screen->Draw();
    vsm_s->StopUseShader();
  } 
  
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


// Singleton
std::unique_ptr<MyGame> my_game;
void init_my_game(){ my_game = std::make_unique<MyGame>(); }
void close_my_game(){ my_game.reset(); }
MyGame *get_my_game(){ return my_game.get(); }
