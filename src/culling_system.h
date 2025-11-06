#pragma once
#include <vector>

#include "job_system/job_scheduler.h"
#include "math/vec.h"

namespace op
{
    class RenderContext;
    class Bounds;
    class RenderComp;
    class Object;
    class CullingBuffer;

    enum class CullingGroup : uint8_t
    {
        COMMON = 0,
        SHADOW = 1,
        TRANSPARENT = 2,

        COUNT = 3
    };

    #define CULLING_BUFFER_COUNT static_cast<uint8_t>(CullingGroup::COUNT)

    class CullingSystem final
    {
    public:
        CullingSystem();
        ~CullingSystem();
        CullingSystem(const CullingSystem& other) = delete;
        CullingSystem(CullingSystem&& other) noexcept = delete;
        CullingSystem& operator=(const CullingSystem& other) = delete;
        CullingSystem& operator=(CullingSystem&& other) noexcept = delete;

        void Cull();

        void DrawConsoleUi();

    private:
        vec<Bounds> m_bounds;

        sl<uint32_t> m_input;
        sl<uint32_t> m_output;
        JobScheduler m_jobScheduler;

        static bool CullOnce(const Bounds& bounds, const std::array<Vec4, 6>& planes);
    };

    class CullingBuffer
    {
    public:
        struct Accessor
        {
            Accessor() = default;
            
            void Submit(cr<Bounds> bounds);
            bool GetVisible() const { assert(m_enable); return *(m_buffer->m_soa.visible.Data() + m_index) != 0.0f; }
            bool IsEnable() const { return m_enable; }
            void Release() { assert(m_enable); m_enable = false; }

        private:
            
            bool m_enable = false;
            uint32_t m_index;
            CullingBuffer* m_buffer;

            friend class CullingBuffer;
        };

        CullingBuffer() = default;
        ~CullingBuffer();
        CullingBuffer(const CullingBuffer& other) = delete;
        CullingBuffer(CullingBuffer&& other) noexcept = delete;
        CullingBuffer& operator=(const CullingBuffer& other) = delete;
        CullingBuffer& operator=(CullingBuffer&& other) noexcept = delete;

        Accessor* Alloc();

        sp<Job> CreateCullJob(cr<arr<Vec4, 6>> planes);
        void WaitForCull();

    private:
        struct CullingSoA
        {
            sl<float> centerX;
            sl<float> centerY;
            sl<float> centerZ;
            sl<float> extentsX;
            sl<float> extentsY;
            sl<float> extentsZ;

            sl<float> visible;

            CullingSoA();

            void Add();
        };
        
        CullingSoA m_soa;
        uint32_t m_firstEmpty = 0;
        vec<Accessor*> m_accessors;
        sp<Job> m_cullJob = nullptr;
        
        void SetBounds(uint32_t index, cr<Bounds> bounds);
        void CullBatch(cr<arr<Vec4, 6>> planes, uint32_t start, uint32_t end);
    };
}
