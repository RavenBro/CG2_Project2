#pragma once

#include <vector>
#include <LiteMath.h>
#include <glad/glad.h>
#include <TinyGLTF/tiny_gltf.h>

namespace renderer
{

using namespace LiteMath;

struct Mesh;
class SceneNode;
class SceneCamera;

struct Material {
    std::string name = "default";
    float roughness = 0.8;
    float metallic = 0.2;
    float reflection = 0;
    GLuint metallicRoughnessTex = 0;
    GLuint albedoTex = 0;
    GLuint normalTex = 0;
    float4 color {1, 1, 1, 1};
};

class Scene {
public:
    std::vector<int> rootNodes;
    std::vector<SceneNode> nodes;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<GLuint> textures;
    std::vector<GLuint> buffers;
    
    struct {
        float4x4 projection;
        float4x4 view;
    } sceneCamera;
    
    Material defaultMaterial;

    void SetCamera(const float4x4 &projection, const float4x4 &view){
        sceneCamera.projection = projection;
        sceneCamera.view = view;
    }

    void SetCamera(const float4x4 &view){
        sceneCamera.view = view;
    }

    bool LoadScene(const char *path, const char *sceneName);
    bool LoadScene(const char *path);
private:
    bool LoadScene(const tinygltf::Model &model, const tinygltf::Scene &scene);
    bool LoadBuffers(const tinygltf::Model &model);
    bool LoadTextures(const tinygltf::Model &model);
    bool LoadMaterials(const tinygltf::Model &model);
    bool LoadMeshes(const tinygltf::Model &model);
    bool LoadNodes(const tinygltf::Model &model);
};

class SceneNode {
public:
    int meshId = -1;
    float4x4 matrix;
    std::string name;
    std::vector<int> children;
};

struct Mesh {
    std::string name;
    struct Primitive {
        int materialId = -1; 
        GLuint vao = 0; 
        bool hasIndexies = false;
        GLenum mode, type;
        GLsizei count = 0;
        void *indexOffset = nullptr;
    
        void Draw() {
            glBindVertexArray(vao);
            if(hasIndexies){
                glDrawElements(mode, count, type, indexOffset);
            } else {
                glDrawArrays(mode, 0, count);
            }
            glBindVertexArray(0);
        }
    };
    std::vector<Primitive> primitives;
};

};