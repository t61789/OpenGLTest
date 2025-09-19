#pragma once

#include "math/math.h"
#include "nlohmann/json.hpp"

#include "i_resource.h"
#include "scene_object_indices.h"

namespace op
{
    class Object;

    class Scene final : public IResource, public std::enable_shared_from_this<Scene>
    {
    public:
        Vec3 ambientLightColorSky = Vec3(0, 0, 0);
        Vec3 ambientLightColorEquator = Vec3(0, 0, 0);
        Vec3 ambientLightColorGround = Vec3(0, 0, 0);
        float tonemappingExposureMultiplier = 1.0f;
        float fogIntensity = 0.01f;
        Vec3 fogColor = Vec3(1.0f, 1.0f, 1.0f);

        Scene() = default;
        ~Scene() override;
        Scene(const Scene& other) = delete;
        Scene(Scene&& other) noexcept = delete;
        Scene& operator=(const Scene& other) = delete;
        Scene& operator=(Scene&& other) noexcept = delete;

        SceneObjectIndices* GetIndices() const { return m_objectIndices.get();}
        cr<StringHandle> GetPath() override { return m_path;}
        sp<Object> GetRoot() const { return m_sceneRoot;}
        
        static sp<Scene> LoadScene(cr<StringHandle> sceneJsonPath);

    private:
        StringHandle m_path;
        
        sp<Object> m_sceneRoot = nullptr;
        up<SceneObjectIndices> m_objectIndices;
        
        void LoadSceneConfig(cr<nlohmann::json> configJson);
        
        static void LoadChildren(crsp<Object> parent, cr<nlohmann::json> children);
    };
}
