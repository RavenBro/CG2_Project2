#include "Renderers.h"
    typedef struct s_node
    {
        uint link;
        uint pixel_pos_on_screen[2];
        float light_pos_projected[2];
    } node;
void IrregularZBufferRenderer::SetTextures(GLuint vertex, GLuint depth)
{
    vertexTex = vertex;
    depthTex = depth;
}
void IrregularZBufferRenderer::SetShaders(ShaderProgram *dc, ShaderProgram *ir, ShaderProgram *sd)
{
    DataConstructor = dc;
    IrregularRasterizer = ir;
    ShadeDrawer = sd;
}
void IrregularZBufferRenderer::Init(int w, int h,ISceneObject *quad)
{
    texWidth = w;
    texHeight = h;
    Quad = quad;

    glGenTextures(1, &shadeTex);
    glBindTexture(GL_TEXTURE_2D, shadeTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, texWidth, texHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
    GL_CHECK_ERRORS;



    glGenFramebuffers(1, &FrameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadeTex, 0);
    GLuint dbuff[]= {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, dbuff);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    #define I_W 800
    #define I_H 600
    #define UINT_SIZE 4 
    #define CLUST 32
    #define NODE_SZ 24
    uint total_size = (I_W*I_H+CLUST)*UINT_SIZE+I_W*I_H*NODE_SZ;
    void *buf = calloc(total_size,1);
    uint *clust_data = (uint *)buf;
    for (int i=0;i<CLUST;i++)
    {
        clust_data[i] = i;
    }
    for (int i=CLUST;i<CLUST+I_W*I_H;i++)
    {
        clust_data[i] = 0;
    }
    glGenBuffers(1, &DataStructure);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, DataStructure);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, DataStructure);
    glBufferData(GL_SHADER_STORAGE_BUFFER, total_size,
                 buf, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GL_CHECK_ERRORS;

    void *buf2 = calloc(I_W*I_H*UINT_SIZE,1);
    uint *depthbuf = (uint *)buf2;
    for (int i=0;i<I_W*I_H;i++)
    {
        depthbuf[i]=~0;
    }
    glGenBuffers(1, &DepthStructure);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, DepthStructure);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, DepthStructure);
    glBufferData(GL_SHADER_STORAGE_BUFFER, I_W*I_H*UINT_SIZE,
                 buf2, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GL_CHECK_ERRORS;
}
void IrregularZBufferRenderer::Render(float4x4 view, float4x4 proj, float4x4 viewinv, float4x4 light)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
    glViewport(0, 0, texWidth, texHeight);
    auto shader = DataConstructor;
    shader->StartUseShader();
    shader->SetUniform("vertex_tex", 0);
    //shader->SetUniform("proj", proj);
    shader->SetUniform("L_Vinv", mul(light,viewinv));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vertexTex);
    Quad->Draw(0);
    /*for(auto obj : GetObjects()){
        for(auto i = 0u; i < obj->GetMeshCount(); i++)
        {
            obj->Draw(i);
        }
    }*/
    shader->StopUseShader();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    shader = IrregularRasterizer;
    shader->StartUseShader();
    for(auto obj : GetObjects()){
        for(auto i = 0u; i < obj->GetMeshCount(); i++)
        {
            shader->SetUniform("MVP",mul(light,obj->GetTransform(i)));
            obj->Draw(i);
        }
    }
    shader->StopUseShader();

    
    
    shader = ShadeDrawer;
    shader->StartUseShader();
    shader->SetUniform("vertex_tex", 0);
    shader->SetUniform("L_Vinv", mul(light,viewinv));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vertexTex);
    Quad->Draw(0);
    shader->StopUseShader();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, DataStructure);
    void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    uint *cnt = (uint *)ptr;
    for (int i=0;i<CLUST;i++)
    {
        //fprintf(stderr,"<%d>",(int)(cnt[i]));
        cnt[i] = i;
    }
    int count = 0;
    for (int i=CLUST;i<CLUST+I_W*I_H;i++)
    {
        if (cnt[i]==0) 
        {count++;
        }
        else cnt[i]=0;
    }
    //fprintf(stderr,"<%d %d>",I_W*I_H-count,count);
    /*
    //SSBO block size
    GLint index = glGetProgramResourceIndex(shader->GetProgram(), GL_SHADER_STORAGE_BLOCK, "data"); //ssboTBTTree - имя лэйаута, см код шейдера.

    static const GLenum ssboProperties[] = { GL_BUFFER_DATA_SIZE };
    GLint ssbo_params[ 1 ] = {};
    glGetProgramResourceiv(shader->GetProgram(), GL_SHADER_STORAGE_BLOCK, index, 1, ssboProperties, 1, NULL, ssbo_params);
    fprintf(stderr,"SSBO DATA BLOCK SIZE: %d\n" ,ssbo_params[0]);
    */
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, DepthStructure);
    ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    cnt = (uint *)ptr;
    count = 0;
    for (int i=0;i<I_W*I_H;i++)
    {
        if (cnt[i]==~0) count++;
        else cnt[i]=~0;
    }
    //fprintf(stderr,"<%d %d>",I_W*I_H-count,count);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GL_CHECK_ERRORS;
}
IrregularZBufferRenderer::~IrregularZBufferRenderer()
{
    glDeleteTextures(1, &shadeTex);
    glDeleteFramebuffers(1, &FrameBuf);
}