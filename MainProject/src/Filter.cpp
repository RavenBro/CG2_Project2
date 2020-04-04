#include "RenderPass.hpp"
#include "Renderer.hpp"

namespace renderer 
{

Filter::~Filter() {
    glDeleteFramebuffers(1, &framebuffer);
}

bool Filter::SetTargetTextures(const std::vector<Filter::TexInfo> &textures){
    std::vector<GLenum> drawTex;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    //std::cout << framebuffer << "\n";
    for(const auto& inf : textures){
        GLuint texture = GenerateTexture2D(inf.fmt, inf.width, inf.height, inf.level);
        std::cout << texture << "\n";
        if(texture == 0){
            throw(std::runtime_error("empty tex attach"));
        }
        
        glFramebufferTexture(GL_FRAMEBUFFER, inf.fbTarget, texture, inf.level);
        drawTex.push_back(inf.fbTarget);
        RegisterTexture(texture, inf.name);
        GL_CHECK_ERRORS;
    }
    glDrawBuffers(drawTex.size(), drawTex.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    /*
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
    case GL_FRAMEBUFFER_UNDEFINED:
        std::cout << "undef\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cout << "incom att\n";
        break;
    case GL_FRAMEBUFFER_COMPLETE:
        std::cout << "win\n";
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cout << "unsup\n";
        break;
    default:
        std::cout << "Other\n";
        break;
    } 
    glBindFramebuffer(GL_FRAMEBUFFER, 0); */
    return glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void Filter::RenderQuad() {
    GetRenderer()->GetScreenQuad()->Draw();
}

};