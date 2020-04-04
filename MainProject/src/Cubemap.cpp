#include "Cubemap.h"
#include "../stb-master/stb_image.h"

static bool load_cube_map_side(
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

static unsigned int loadCubemap(const std::vector<std::string>& faces)
{
      glActiveTexture(GL_TEXTURE0);
      unsigned int textureID;
      glGenTextures(1, &textureID);
      glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

      
      // load each image and copy into a side of the cube-map texture
      bool ok = true;
      ok = ok && load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, faces[0].c_str());
      ok = ok && load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, faces[2].c_str());
      ok = ok && load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, faces[5].c_str());
      ok = ok && load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, faces[1].c_str());
      ok = ok && load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, faces[4].c_str());
      ok = ok && load_cube_map_side(textureID, GL_TEXTURE_CUBE_MAP_POSITIVE_X, faces[3].c_str());
      if(!ok) return 0;
      // format cube map texture
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
      return textureID;
}

bool Skybox::Load(const std::vector<std::string>& faces)
{
    texId = loadCubemap(faces);
    if(!texId) return false;
    baseCubemap.reset(CreateSkyBoxMesh());
    return true;
}

void Skybox::Draw() 
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, texId);
	baseCubemap->Draw();
}