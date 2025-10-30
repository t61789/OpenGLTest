#pragma once
#include "const.h"
#include "i_render_pass.h"
#include "math/matrix4x4.h"
#include "objects/transform_comp.h"

namespace op
{
    class RenderTexture;
    class Material;

    class MainLightShadowPass : public IRenderPass
    {
    public:
        explicit MainLightShadowPass();
        ~MainLightShadowPass() override;
        std::string GetName() override;
        void Execute() override;

    private:
        void UpdateRt();
        
        static void CalcShadowMatrix(TransformComp* lightTrans, TransformComp* cameraTrans, Matrix4x4& viewMatrix, Matrix4x4& projMatrix, Vec3& shadowCenter);

        sp<Material> m_drawShadowMat = nullptr;
        sp<RenderTexture> m_mainLightShadowRt = nullptr;
    };
}
