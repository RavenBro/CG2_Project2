
class OutputController
{
public:
enum DrawStates {NORMAL = 0157};
enum DrawOnlyStates {LIGHT = 1, REFLECTIONS = 2, SHADOWS = 4, MATERIAL = 8, BLOOM = 16, SSAO = 32, COLOR =64};
enum ShadowsTypes {NONE, SIMPLE, VSM, CASCADED, CUBEMAP};
void InvertPos(int pos)
{
    pos = 1<<pos;
    if (draw_flags&pos) draw_flags -=pos;
    else draw_flags +=pos;
}
void SetNextShadowsType()
{
    shadows_type=(shadows_type+1)%5;
}
void SetNormalDraw()
{
    draw_flags = NORMAL;
}
void SetShadowsType(int type)
{
    shadows_type = type;
}
int GetDrawFlags()
{
    return(draw_flags);
}
int GetShadowsType()
{
    return(shadows_type);
}
void GetDrawArray(int buf[7])
{
    for (int i=0;i<7;i++)
    {
        buf[i]=(draw_flags>>i)%2;
    }
}
void SetDrawFlags(unsigned flags)
{
    draw_flags=flags;
}
private:
unsigned draw_flags=0, shadows_type=0;
};