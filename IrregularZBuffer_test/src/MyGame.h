#pragma once
#include <memory>
#include "framework/Game.h"
#include "framework/Objects3d.h"
#include "framework/Primitives.h"
#include "framework/ShaderProgram.h"
#include "framework/HelperGL.h"

#include "MyCameras.h"
#include "Scene.h"
#include "IndoorScene.h"
#include "ShadowMap.h"
#include <math.h>
typedef ShaderProgram Shader;

class MyGame
{
  // feel free to
  // improve this
  ColorMesh *model;
  GameWindow* gw;
  FreeCamera* fc;
  TankCamera* tc;
  Mesh* triangle;
  Mesh* box_targ;
  Mesh* plane_gr;
  Mesh* skybox;
  Model* my_tank;
  Model* my_stealth_tank;
  Shader* grid_s;
  Shader* simp_s;
  Shader* targ_s;
  Shader* tank_s;
  Shader* skyb_s;
  Shader* mirr_s;
  Shader* depthbuff_s;
  Shader* shadowmap_s;
  Shader* default_s;
  Shader* vsm_s;
  Shader* vsm_buff_draw_s;
  Shader* gaucean_s;
  Shader *SSAO_s;
  Shader *SSAO_visualizer_s;
  // links to res
  Camera* my_cam;
  ShadowMap *shadowmap;
  VarianceShadowMap *vshadowmap;
  bool drawDepthBuff = false;
  Mesh* screen;
  std::unique_ptr<TextureMap> texMap;
  std::unique_ptr<PostEffectsMap> postEffectsMap;
  FrameBuffer *filter_fb;

  std::vector<float4x4> box_model;
  // you can use smart pointers for the code above

  class ShaderZone //Simple scope guard
  {
    MyGame *parent;
    Shader *shader;
  public:
    ShaderZone(MyGame* g, Shader* shader);
    ~ShaderZone();
  };
  float4x4 light;
  float FOVAngles;
  float ratio;
  float MinDistance;
  float MaxDistance;
  float4x4 projection;
  void update(const GameState &gs);
  void draw(const GameState &gs);
  void createShadows();
  void createBlur();
  void createBuffer();
  void calculate_SSAO();
  void setGSigma(float s);

  Scene scene;
  //IndoorScene indoor_scene;
public:
  MyGame();
  ~MyGame();
  void start();
};

// Singleton
void init_my_game();
void close_my_game();
MyGame *get_my_game();

// Singleton means that there can be only one instance for all program. (single)
// MyGame is the exactly this case.
// So you can get acces to MyGame instance from any part of this code just by including "MyGame.h"
