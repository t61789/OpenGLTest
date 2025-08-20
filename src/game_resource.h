#pragma once
#include "utils.h"

#define GET_GLOBAL_CBUFFER (GameResource::Ins()->GetPredefinedMaterial(GLOBAL_CBUFFER))

struct GLFWwindow;
namespace op
{
    class Material;
    
    class GameResource : public Singleton<GameResource>
    {
    public:
        Time time;
        Event<> onFrameEnd;
        Event<GLFWwindow*, int, int> onFrameBufferResize;

        GameResource();
        ~GameResource();

        void SubmitPredefinedMaterial(size_t nameId, Material* material);
        Material* GetPredefinedMaterial(size_t nameId);

    private:
        std::unordered_map<size_t, Material*> m_predefinedMaterials;
    };
}
