#pragma once
#include <vector>

#include "math/vec.h"

namespace op
{
    class RenderContext;
    class Bounds;
    class RenderComp;
    class Object;
    class CullingBuffer;

    class CullingSystem final
    {
    public:
        CullingSystem();
        ~CullingSystem();
        CullingSystem(const CullingSystem& other) = delete;
        CullingSystem(CullingSystem&& other) noexcept = delete;
        CullingSystem& operator=(const CullingSystem& other) = delete;
        CullingSystem& operator=(CullingSystem&& other) noexcept = delete;

        CullingBuffer* GetCullingBuffer() const { return m_cullingBuffer; }

        void Cull();

        void DrawConsoleUi();

    private:
        vec<Bounds> m_bounds;
        CullingBuffer* m_cullingBuffer = nullptr;

        bool CullOnce(const Bounds& bounds, const std::array<Vec4, 6>& planes);
    };


    

    struct CullingBufferAccessor
    {
        CullingBufferAccessor() = default;
        CullingBufferAccessor(uint32_t index, CullingBuffer* buffer);

        void Submit(cr<Bounds> bounds);
        bool GetVisible();
        bool IsEnable() const { return m_enable; }

    private:
        bool m_enable = false;
        uint32_t m_index;
        CullingBuffer* m_buffer;

        friend class CullingBuffer;
    };




    class CullingBuffer
    {
    public:
        CullingBufferAccessor Alloc();
        void Release(CullingBufferAccessor& accessor);

        void SetBounds(uint32_t index, cr<Bounds> bounds);
        bool GetVisible(uint32_t index);

        void Cull(cr<arr<Vec4, 6>> planes);

    private:
        struct ElemInfo
        {
            bool empty = true;
        };
    
        struct CullingSoA
        {
            vec<float> centerX;
            vec<float> centerY;
            vec<float> centerZ;
            vec<float> extentsX;
            vec<float> extentsY;
            vec<float> extentsZ;
            vec<float> visible;
        };
        
        CullingSoA m_buffer;
        uint32_t m_firstEmpty = 0;
        vec<ElemInfo> m_elemInfos;
    };
}
