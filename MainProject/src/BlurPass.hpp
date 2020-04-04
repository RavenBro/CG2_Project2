#pragma once

#include "RenderPass.hpp"

namespace renderer
{

class BlurPass : public RenderPass {
public:
    virtual ~BlurPass() { Clear(); }
    void OnInit() override;
    void Process() override;
    void SetTexture(GLuint texture, const GLTexture2DFormat &fmt, int width, int height, int level);
    void ResetTexture(GLuint texture, int level);
    void Clear();
    void DeleteTempTex();
    void SetSigma(float sg);
private:
    std::vector<float> coef;
    GLuint fbpass1 = 0, fbpass2 = 0;
    GLuint targetTex = 0, tempTex = 0;
    int texW = 0, texH = 0;

    struct {
        GLint srcTex = -1, isHorizontal = -1, weight = -1, weights = -1;  
    } uniforms;
};

};