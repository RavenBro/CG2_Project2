#include "Scenev2.hpp"
#include <memory>
#include <iostream>
#include "framework/HelperGL.h"

namespace renderer 
{

static std::unique_ptr<tinygltf::TinyGLTF> loader;

static std::string GetFilePathExtension(const std::string &FileName) {
  if (FileName.find_last_of(".") != std::string::npos)
    return FileName.substr(FileName.find_last_of(".") + 1);
  return "";
}

static size_t ComponentTypeByteSize(int type) {
  switch (type) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      return sizeof(char);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      return sizeof(short);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    case TINYGLTF_COMPONENT_TYPE_INT:
      return sizeof(int);
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      return sizeof(float);
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
      return sizeof(double);
    default:
      return 0;
  }
}

static bool LoadModel(const char *path, tinygltf::Model *model, bool showLog = true)
{
    if (!loader)
    loader = std::make_unique<tinygltf::TinyGLTF>();
    std::string err;
    std::string warn;

    std::string input_filename(path);
    std::string ext = GetFilePathExtension(input_filename);
    bool status = false;
    if(ext == "glb") {
        status = loader->LoadBinaryFromFile(model, &err, &warn, input_filename);
    } else if(ext == "gltf") {
        status = loader->LoadASCIIFromFile(model, &err, &warn, input_filename);
    } else {
        err += "Unsupported file format " + ext + "\n";
    }
    if(showLog){
        if(err.size()) std::cout << "LoadModel errors : " << err << "\n";
        if(warn.size()) std::cout << "LoadModel warnings : " << warn << "\n"; 
    }
    return status;
}

bool Scene::LoadScene(const char *path, const char *sceneName)
{
    tinygltf::Model model;
    if(!LoadModel(path, &model)){
        return false;
    }
    for(const auto &scene : model.scenes){
        if(scene.name == sceneName){
            return LoadScene(model, scene);
        }
    }
    std::cout << "Scene " << sceneName << " not found \n";
    return false;
}
bool Scene::LoadScene(const char *path)
{
    tinygltf::Model model;
    if(!LoadModel(path, &model)){
        return false;
    }

    if(model.defaultScene < 0){
        std::cout << "Default scene not found \n";
        return false;
    }
    return LoadScene(model, model.scenes[model.defaultScene]);
}

bool Scene::LoadScene(const tinygltf::Model &model, const tinygltf::Scene &scene){
    if(!Scene::LoadBuffers(model)){
        return false;
    }
    if(!Scene::LoadTextures(model)){
        return false;
    }
    if(!Scene::LoadMaterials(model)){
        return false;
    }
    if(!Scene::LoadMeshes(model)){
        return false;
    }
    if(!Scene::LoadNodes(model)){
        return false;
    }
    rootNodes = scene.nodes;
    return true;
}

bool Scene::LoadBuffers(const tinygltf::Model &model){
    for(const auto& accessor : model.accessors){
        if(accessor.sparse.isSparse){
            std::cout << "tiny_gltf docs : sparse accessors are not supported\n";
            return false;
        }
        const tinygltf::BufferView &view = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[view.buffer];

        GLuint buffId;
        glGenBuffers(1, &buffId);
        glBindBuffer(view.target, buffId);
        glBufferData(view.target, view.byteLength, &buffer.data.at(0) + view.byteOffset, GL_STATIC_DRAW);
        try { 
            GL_CHECK_ERRORS;
        } 
        catch(std::runtime_error err){
            std::cout << err.what() << "\n";
            return false;
        }
        buffers.push_back(buffId);
    }
    return true;
}

static GLenum gltfTexEnumToGL(int gltfTex){
    switch (gltfTex)
    {
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
        return GL_LINEAR;
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
        return GL_NEAREST;
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
        return GL_LINEAR_MIPMAP_LINEAR;
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
        return GL_LINEAR_MIPMAP_NEAREST;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
        return GL_NEAREST_MIPMAP_NEAREST;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
        return GL_CLAMP_TO_EDGE;
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
        return GL_REPEAT;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
        return GL_MIRRORED_REPEAT;
    default:
        std::cout << "Unsupported enum \n";
        return 0;
    }
}

bool Scene::LoadTextures(const tinygltf::Model &model){
    for(const auto& texture : model.textures){
        if(texture.source < 0){
            textures.push_back(0);
            continue;
        }
        const tinygltf::Image &img = model.images[texture.source];
        if(img.width < 0 || img.height < 0){
            std::cout << "WARN: Image: '" << img.uri << "' is not loaded." << std::endl;
            textures.push_back(0);
            continue;
        }

        tinygltf::Sampler sampler;
        sampler.minFilter = (sampler.minFilter == -1)? TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR : sampler.minFilter;
        sampler.magFilter = (sampler.magFilter == -1)? TINYGLTF_TEXTURE_FILTER_LINEAR : sampler.magFilter;
        if(texture.sampler < 0){
            sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
            sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
            sampler.wrapR = sampler.wrapS = sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
        }
        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gltfTexEnumToGL(sampler.magFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gltfTexEnumToGL(sampler.minFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gltfTexEnumToGL(sampler.wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gltfTexEnumToGL(sampler.wrapT));

        GLenum format = GL_RGBA;
        if (img.component == 1) {
          format = GL_RED;
        } else if (img.component == 2) {
          format = GL_RG;
        } else if (img.component == 3) {
          format = GL_RGB;
        } else {

        }

        GLenum type = GL_UNSIGNED_BYTE;
        if (img.bits == 8) {
          // ok
        } else if (img.bits == 16) {
          type = GL_UNSIGNED_SHORT;
        } else {
          // ???
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, format, type, &img.image.at(0));
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        try { 
            GL_CHECK_ERRORS;
        } 
        catch(std::runtime_error err){
            std::cout << err.what() << "\n";
            return false;
        }
        textures.push_back(texId);
    }
    return true;
}

bool Scene::LoadMaterials(const tinygltf::Model &model){
    for(const auto& material : model.materials){
        Material resMT;
        resMT.name = material.name;
        resMT.roughness = material.pbrMetallicRoughness.roughnessFactor;
        resMT.metallic = material.pbrMetallicRoughness.metallicFactor;
        if(material.pbrMetallicRoughness.baseColorFactor.size() == 4){
            const auto &v = material.pbrMetallicRoughness.baseColorFactor;
            resMT.color = float4(v[0], v[1], v[2], v[3]);
        }

        int i = material.pbrMetallicRoughness.baseColorTexture.index;
        resMT.albedoTex = (i >= 0) ? textures[i] : 0;
        i = material.normalTexture.index;
        resMT.normalTex = (i >= 0) ? textures[i] : 0;
        i = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        resMT.metallicRoughnessTex = (i >= 0) ? textures[i] : 0;
        materials.push_back(resMT);
    }
    return  true;
}

bool Scene::LoadMeshes(const tinygltf::Model &model){
    for(const auto& mesh : model.meshes){
        Mesh resM;
        resM.name = mesh.name;
        for(const auto &prim : mesh.primitives){
            Mesh::Primitive resPrim;
            GLuint vao;
            glGenVertexArrays(1, &vao);
            GL_CHECK_ERRORS;
            assert(vao != 0);
            glBindVertexArray(vao);
            GLuint attrib_n = 4;
            int position_acc = -1;
            for(const auto &pair : prim.attributes){
                if(pair.second < 0) return false;
            
                const tinygltf::Accessor accessor = model.accessors[pair.second];
                const tinygltf::BufferView &view = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[view.buffer];
                GLuint vbo = buffers[pair.second];
                glBindBuffer(view.target, vbo);
                int comp_num = tinygltf::GetNumComponentsInType(accessor.type);        
                GLuint attr;
                if(pair.first == "POSITION"){
                    position_acc = pair.second;
                    attr = 0;
                } else if(pair.first == "NORMALS"){
                    attr = 1;
                } else if(pair.first == "TEXCOORD_0"){
                    attr = 2;
                } else if(pair.first == "TEXCOORD_1"){
                    attr = 3;
                } else {
                    attr = attrib_n;
                    attrib_n++;
                }
                int bs = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
                glVertexAttribPointer(attr, comp_num, accessor.componentType, accessor.normalized, bs, 
                    (char*)nullptr + accessor.byteOffset);   
                GL_CHECK_ERRORS;
                glEnableVertexAttribArray(attr);
                GL_CHECK_ERRORS;     
            }

            switch(prim.mode){
                case TINYGLTF_MODE_TRIANGLES:
                    resPrim.mode = GL_TRIANGLES;
                    break;
                case TINYGLTF_MODE_TRIANGLE_FAN:
                    resPrim.mode = GL_TRIANGLE_FAN;
                    break;
                case TINYGLTF_MODE_TRIANGLE_STRIP:
                    resPrim.mode = GL_TRIANGLE_STRIP;
                    break;
                case TINYGLTF_MODE_POINTS:
                    resPrim.mode = GL_POINTS;
                    break;
                case TINYGLTF_MODE_LINE:
                    resPrim.mode = GL_LINES;
                    break;
                case TINYGLTF_MODE_LINE_LOOP:
                    resPrim.mode = GL_LINE_LOOP;
                    break;
                case TINYGLTF_MODE_LINE_STRIP:
                    resPrim.mode = GL_LINE_STRIP;
                    break;
                default:
                    assert("Unimplimented" == nullptr);
            }

            if(prim.indices >= 0){
                resPrim.hasIndexies = true; 
                const auto &acc = model.accessors[prim.indices];
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[prim.indices]);
                resPrim.count = acc.count;
                resPrim.type = acc.componentType;
                resPrim.indexOffset = (char*)nullptr + acc.byteOffset;
            } else {
                resPrim.hasIndexies = false;
                if(position_acc < 0) return false;
                const auto &acc = model.accessors[position_acc];
                resPrim.count = acc.count;
            }
            glBindVertexArray(0);
            resPrim.vao = vao;
            resPrim.materialId = (prim.material >= 0)? prim.material : -1;
            try { 
                GL_CHECK_ERRORS;
            } 
            catch(std::runtime_error err){
                std::cout << err.what() << "\n";
                return false;
            }
            resM.primitives.push_back(resPrim);
        }
        meshes.push_back(resM);
    }
    return true;
}

static float4x4 quanternion_rot(const float4 &q){
    float4x4 m;
    m.identity();
    m.M(0, 0) = 1 - 2*q.y*q.y - 2*q.z*q.z;
    m.M(0, 1) = 2*q.x*q.y - 2*q.z*q.w;
    m.M(0, 2) = 2*q.x*q.z + 2*q.y*q.w;
    m.M(1, 0) = 2*q.x*q.y + 2*q.z*q.w;
    m.M(1, 1) = 1 - 2*q.x*q.x - 2*q.z*q.z;
    m.M(1, 2) = 2*q.y*q.z - 2*q.x*q.w;
    m.M(2, 0) = 2*q.x*q.z - 2*q.y*q.w;
    m.M(2, 1) = 2*q.y*q.z + 2*q.x*q.w;
    m.M(2, 2) = 1 -2*q.x*q.x - 2*q.y*q.y;
    return m;
}

bool Scene::LoadNodes(const tinygltf::Model &model)
{
    for(const auto &node : model.nodes){
        SceneNode snode;
        snode.name = node.name;
        snode.meshId = node.mesh;
        snode.children = node.children;
        if(node.matrix.size() == 16){
            float arr[16];
            for(int i = 0; i < 16; i++) arr[i] = node.matrix[i];
            snode.matrix = transpose4x4(float4x4(arr));
        } else {
            float3 scale {1, 1, 1};
            float4 rot {0, 0, 0, 0};
            float3 trans {0, 0, 0};
            if(node.scale.size() == 3){
                scale = float3(node.scale[0], node.scale[1], node.scale[2]);
            }
            if(node.translation.size() == 3){
                trans = float3(node.translation[0], node.translation[1], node.translation[2]);
            }
            if(node.rotation.size() == 4){
                rot = float4(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]);
            }

            snode.matrix = mul(translate4x4(trans), mul(quanternion_rot(rot), scale4x4(scale)));
        }
        nodes.push_back(snode);
    }
    return true;
}

};