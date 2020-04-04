#include "MyGame.h"
#include <memory>
#include <algorithm>
#include <cmath>
#include <glad/glad.h>
#include <iostream>
#include <random>
#include "../stb-master/stb_image.h"

//Roman's shit
  static void Show(float4x4 matr) {
      for (int i = 0; i < 4; i++)
      {
          std::cout<<matr.row[i].x<<" ";
          std::cout<<matr.row[i].y<<" ";
          std::cout<<matr.row[i].z<<" ";
          std::cout<<matr.row[i].w<<" ";
          std::cout<<std::endl;
      }
      
  }

bool load_cube_map_side(
    GLuint texture, GLenum side_target, const char* file_name) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int x, y, n;
    int force_channels = 4;
    unsigned char*  image_data = stbi_load(
      file_name, &x, &y, &n, force_channels);
    if (!image_data) {
      fprintf(stderr, "ERROR: could not load %s\n", file_name);
      return false;
    }
    // non-power-of-2 dimensions check
    if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
      fprintf(stderr,
        "WARNING: image %s is not power-of-2 dimensions\n",
        file_name);
    }
    
    // copy image data into 'target' side of cube map
    glTexImage2D(
      side_target,
      0,
      GL_RGBA,
      x,
      y,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      image_data);
    free(image_data);
    return true;
}

  unsigned int loadCubemap(vector<std::string> faces)
  {
      glActiveTexture(GL_TEXTURE0);
      unsigned int textureID;
      glGenTextures(1, &textureID);
      glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

      
      // load each image and copy into a side of the cube-map texture
      load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, faces[0].c_str());
      load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, faces[2].c_str());
      load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, faces[5].c_str());
      load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, faces[1].c_str());
      load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, faces[4].c_str());
      load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_POSITIVE_X, faces[3].c_str());
      // format cube map texture
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
      return textureID;
  }

MyGame::MyGame()
{
  gw = new GameWindow(800, 600, "CG/2 Task 2. 2019");
  fc = new FreeCamera();
  tc = new TankCamera();
  fc->setPos(float3(0, 1.0, 5));
  my_cam = fc;
  
  FOVAngles = 90;
  MaxDistance = 1000;
  MinDistance = 0.1;
  ratio = double(gw->getViewWidth())/gw->getViewHeight();
  projection = transpose(projectionMatrixTransposed(FOVAngles,ratio,MinDistance,MaxDistance));

  //scene.SetViewProj(projection, fc);
  //scene.LoadData();
  //scene.SetRenderQueues();
  indoor_scene.SetViewProj(projection, fc);
  indoor_scene.LoadData();
  indoor_scene.SetRenderQueues();
  /*
  plane_gr = CreateSimplePlaneMesh();
  box_targ = CreateSimpleBoxMesh();

  skybox = CreateSkyBoxMesh();
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

  mirr_s = new ShaderProgram({{GL_VERTEX_SHADER, "shaders/RussianStealthTank/vertex.glsl"},
      {GL_FRAGMENT_SHADER, "shaders/RussianStealthTank/fragment.glsl"}});
  GL_CHECK_ERRORS;


  skyb_s = new ShaderProgram({{GL_VERTEX_SHADER, "shaders/SkyBox/vertex.glsl"},
      {GL_FRAGMENT_SHADER, "shaders/SkyBox/fragment.glsl"}});
  GL_CHECK_ERRORS;

  my_tank = new GLTFModel("assets/Tank_M60A1_opt/tank_m60a1.gltf");
  float3 t_pos = float3(-11,0,-4);
  my_tank->transform = mul(my_tank->transform, translate4x4(t_pos));
  my_stealth_tank = new GLTFModel("assets/Tank_M60A1_opt/tank_m60a1.gltf");
  my_stealth_tank->transform = mul(my_stealth_tank->transform, translate4x4(float3(0,0,0)));
  GL_CHECK( glGenVertexArrays(1, &my_stealth_tank->vao));

  // Tank model consists of {"Head", "Body"} meshes
  const auto &it = std::find_if(my_tank->GetMeshes().begin(), my_tank->GetMeshes().end(), [](Mesh *m) {
      return m->GetName() == "Head";
  });
  if (it != my_tank->GetMeshes().end()){
      tc->setPos(to_float3(mul((*it)->GetTransform(),translate4x4(t_pos).get_col(3)))); // set camera to tank Head pos
  }

  //skybox
  vector<std::string> faces{
      "assets/Skybox/hills_bk.tga",
      "assets/Skybox/hills_dn.tga",
      "assets/Skybox/hills_ft.tga",
      "assets/Skybox/hills_lf.tga",
      "assets/Skybox/hills_rt.tga",
      "assets/Skybox/hills_up.tga"
  };
  unsigned int cubemapTexture = loadCubemap(faces);
  glDepthMask(GL_FALSE);
  PrimitiveMesh *a = (PrimitiveMesh *)skybox;
  glBindVertexArray(a->vao_cpy);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  glBindVertexArray(0);
  glDepthMask(GL_TRUE);
  
  glBindVertexArray(my_stealth_tank->vao);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  glBindVertexArray(0);

  //Projection Matrix
  FOVAngles = 90;
  MaxDistance = 1000;
  MinDistance = 0.1;
  ratio = double(gw->getViewWidth())/gw->getViewHeight();
  projection = projectionMatrixTransposed(FOVAngles,ratio,MinDistance,MaxDistance);

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

// Reflection Gatherer
  float3 ProbePos = float3(10, 1, 4);
  //Projection Matrix
  FOVAngles = 90;
  MaxDistance = 1000;
  MinDistance = 0.1;
  ratio = 1;
  float4x4 ProbeProjection = projectionMatrixTransposed(FOVAngles,ratio,MinDistance,MaxDistance);
  my_cam->setPos(ProbePos);
  my_cam->rotateYPR(float3(0,90,0)); */

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
  glEnable(GL_DEPTH_TEST);
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

void MyGame::draw(const GameState &gs)
{
  //scene.Render();
  indoor_scene.Render();
}
std::unique_ptr<MyGame> my_game;
void init_my_game(){ my_game = std::make_unique<MyGame>(); }
void close_my_game(){ my_game.reset(); }
MyGame *get_my_game(){ return my_game.get(); }
