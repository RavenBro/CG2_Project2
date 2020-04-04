#include "MyGame.h"
#include <memory>
#include <algorithm>
#include <cmath>
#include <glad/glad.h>
#include <iostream>
#include <random>
#include "../stb-master/stb_image.h"


MyGame::MyGame()
{
  gw = new GameWindow(800, 600, "CG/2 Task 2. 2019");
  fc = new FreeCamera();
  tc = new TankCamera();
  fc->setPos(float3(2, 1.0, 9.5));
  tc->setPos(float3(0, 1.0, 0));
  tc->rotateYPR(float3(-M_PI/2,0.0,0.0));
  my_cam = fc;
  FOVAngles = 90;
  MaxDistance = 100;
  MinDistance = 0.1;
  ratio = double(gw->getViewWidth())/gw->getViewHeight();
  projection = transpose(projectionMatrixTransposed(FOVAngles,ratio,MinDistance,MaxDistance));

  scene.SetViewProj(projection, my_cam);
  scene.LoadData();
  scene.SetRenderQueues();

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
  {
    my_cam = (my_cam == fc) ? (Camera*)tc : (Camera*)fc;
    scene.SetViewProj(projection, my_cam);
  }

  if (key_released(gs, GLFW_KEY_SPACE))
  {
    float3 pos = my_cam->getPos();
    std::cout << "Camera pos: " << pos.x << ' ' << pos.y << ' ' << pos.z << std::endl;
  }

  if(key_released(gs, GLFW_KEY_1))
  {
    //scene.oc.InvertPos(0);
    my_cam->setPos(float3(2,1.0,9.5));
  }
  if(key_released(gs, GLFW_KEY_2))
  {
    //scene.oc.InvertPos(1);  
    my_cam->setPos(float3(4,1.0,9.5));
  }
  if(key_released(gs, GLFW_KEY_3))
  {
    //scene.oc.InvertPos(2);  
    my_cam->setPos(float3(5,1.0,7.5));
  }
  if(key_released(gs, GLFW_KEY_4))
  {
    //scene.oc.InvertPos(3);  
    my_cam->setPos(float3(7,1.0,7));
  }
  if(key_released(gs, GLFW_KEY_5))
  {
    //scene.oc.InvertPos(4);  
    my_cam->setPos(float3(8,1.0,0));
  }
  if(key_released(gs, GLFW_KEY_6))
  {
    //scene.oc.InvertPos(5);  
    my_cam->setPos(float3(-5,1.0,0));
  }
  if(key_released(gs, GLFW_KEY_7))
  {
    //scene.oc.InvertPos(6);
    my_cam->setPos(float3(0,1.0,-3)); 
  }
  if(key_released(gs, GLFW_KEY_N))
  {
    scene.oc.SetNormalDraw(); 
  }
  if(key_released(gs, GLFW_KEY_P))
  {
    scene.oc.SetNextShadowsType();
  }
  if (my_cam==tc)
  {
      if(key_pressed(gs, GLFW_KEY_R))
      {
        scene.MoveTank(0.01,0,0);
        tc->rotateYPR(float3(0.01,0.0,0.0));  
      }
      if(key_pressed(gs, GLFW_KEY_T))
      {
        scene.MoveTank(-0.01,0,0);
        tc->rotateYPR(float3(-0.01,0.0,0.0));
      }
      if(key_pressed(gs, GLFW_KEY_W))
      {
        float3 pos = scene.MoveTank(0,0,0.02);
        tc->setPos(pos+float3(0.0,1.0,0.0));
      }
      if(key_pressed(gs, GLFW_KEY_S))
      {
        float3 pos = scene.MoveTank(0,0,-0.02);
        tc->setPos(pos+float3(0.0,1.0,0.0));
      }
      if(key_pressed(gs, GLFW_KEY_A))
      {
        float3 pos = scene.MoveTank(0,0.01,0.0);
        tc->setPos(pos+float3(0.0,1.0,0.0));
      }
      if(key_pressed(gs, GLFW_KEY_D))
      {
        float3 pos = scene.MoveTank(0,-0.01,0.0);
        tc->setPos(pos+float3(0.0,1.0,0.0));
      }

  }
  if (mbtn_released(gs, GLFW_MOUSE_BUTTON_RIGHT))
    glfwSetInputMode(gw->get(), GLFW_CURSOR,
                     gs.cursorCaptured ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

  my_cam->update(gs);
}

void MyGame::draw(const GameState &gs)
{
  scene.Render();
}
std::unique_ptr<MyGame> my_game;
void init_my_game(){ my_game = std::make_unique<MyGame>(); }
void close_my_game(){ my_game.reset(); }
MyGame *get_my_game(){ return my_game.get(); }
