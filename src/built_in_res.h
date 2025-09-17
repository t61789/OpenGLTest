#pragma once
#include "utils.h"

namespace op
{
    class Mesh;
    class Material;
    class Image;
    class Shader;

    class BuiltInRes : public Singleton<BuiltInRes>
    {
    public:
        BuiltInRes();

        sp<Mesh> quadMesh = nullptr;
        sp<Mesh> sphereMesh = nullptr;

        sp<Material> blitMatNew = nullptr;

        sp<Image> errorTex = nullptr;
        sp<Image> whiteTex = nullptr;
        sp<Image> blackTex = nullptr;
        sp<Image> missTex = nullptr;

    private:
        vecsp<Shader> m_packedShaders;
        
        void LoadPackedShaders();
    };

    inline BuiltInRes* GetBR()
    {
        return BuiltInRes::Ins();
    }
}
