#include "culling_system.h"

#include <array>

#include <tracy/Tracy.hpp>

#include "render_context.h"
#include "bounds.h"
#include "game_resource.h"
#include "object.h"
#include "scene.h"
#include "objects/batch_render_comp.h"
#include "objects/render_comp.h"
#include "objects/transform_comp.h"

// bool FrustumCulling(const float3 center, const float3 extents)
// {
//     bool result = true;
//     for (uint i = 0; i < 6; i++)
//     {
//         float4 plane = _Planes[i];
//         float3 normal = plane.xyz;
//         float3 s = sign(normal);
//         float3 offset = extents * s;
//         result = result && (
//             dot(center + offset, normal) >= plane.w ||
//             dot(center - offset, normal) >= plane.w);
//     }
//
//     return result;
// }

namespace op
{
    using namespace std;

    CullingSystem::CullingSystem()
    {
        m_cullingBuffer = new CullingBuffer();

        m_input = sl<uint32_t>(10000000);
        m_input.Resize(m_input.Capacity());
        m_output = sl<uint32_t>(m_input.Size());
        m_output.Resize(m_output.Capacity());

        for (uint32_t i = 0; i < m_input.Size(); i++)
        {
            m_input[i] = i;
        }
    }

    CullingSystem::~CullingSystem()
    {
        delete m_cullingBuffer;
    }

    void CullingSystem::Cull()
    {
        ZoneScoped;
        
        // 把visibleRenderObjs扔了，剔除allRenderObjs生成一个新的
        GetRC()->visibleRenderObjs.clear();
    
        const auto& renderObjs = *GetRC()->allRenderObjs;
        auto planes = get_frustum_planes(GetRC()->vpMatrix);

        for (const auto& renderObj : renderObjs)
        {
            // if (CullOnce(renderObj->GetWorldBounds(), planes))
            // {
                GetRC()->visibleRenderObjs.push_back(renderObj.lock().get());
            // }
        }

        m_cullingBuffer->Cull(planes);
    }

    void CullingSystem::DrawConsoleUi()
    {
        // for (auto bounds : m_bounds)
        // {
        //     Utils::DebugDrawCube(
        //         bounds,
        //         m_renderContext->vMatrix,
        //         m_renderContext->pMatrix,
        //         vec2(m_renderContext->screenWidth, m_renderContext->screenHeight));
        // }
    }

    bool CullingSystem::CullOnce(const Bounds& bounds, const array<Vec4, 6>& planes)
    {
        return frustum_culling(planes, bounds.center, bounds.extents);
    }

    
    CullingBufferAccessor::CullingBufferAccessor(const uint32_t index, CullingBuffer* buffer)
    {
        m_index = index;
        m_buffer = buffer;
        m_enable = true;
    }

    void CullingBufferAccessor::Submit(cr<Bounds> bounds)
    {
        m_buffer->SetBounds(m_index, bounds);
    }

    bool CullingBufferAccessor::GetVisible()
    {
        return m_buffer->GetVisible(m_index);
    }

    
    CullingBufferAccessor CullingBuffer::Alloc()
    {
        for (auto searchIndex = m_firstEmpty; searchIndex <= m_elemInfos.size(); ++searchIndex)
        {
            if (searchIndex == m_elemInfos.size())
            {
                for (uint32_t i = 0; i < 4; ++i)
                {
                    if (m_buffer.centerX.Size() == m_buffer.centerX.Capacity())
                    {
                        m_buffer.centerX.Reserve(m_buffer.centerX.Capacity() * 2);
                        m_buffer.centerY.Reserve(m_buffer.centerY.Capacity() * 2);
                        m_buffer.centerZ.Reserve(m_buffer.centerZ.Capacity() * 2);
                        m_buffer.extentsX.Reserve(m_buffer.extentsX.Capacity() * 2);
                        m_buffer.extentsY.Reserve(m_buffer.extentsY.Capacity() * 2);
                        m_buffer.extentsZ.Reserve(m_buffer.extentsZ.Capacity() * 2);
                        m_buffer.visible.Reserve(m_buffer.visible.Capacity() * 2);
                    }
                    
                    m_buffer.centerX.Add(0);
                    m_buffer.centerY.Add(0);
                    m_buffer.centerZ.Add(0);
                    m_buffer.extentsX.Add(0);
                    m_buffer.extentsY.Add(0);
                    m_buffer.extentsZ.Add(0);
                    m_buffer.visible.Add(1);

                    m_elemInfos.push_back({});
                }
            }
            
            if (m_elemInfos[searchIndex].empty)
            {
                m_firstEmpty = searchIndex + 1;
                m_elemInfos[searchIndex].empty = false;
                return { searchIndex, this };
            }
        }

        throw std::runtime_error("CullingBuffer::Alloc: This could not happened");
    }

    void CullingBuffer::Release(CullingBufferAccessor& accessor)
    {
        assert(!m_elemInfos[accessor.m_index].empty);

        m_elemInfos[accessor.m_index].empty = true;
        accessor.m_enable = false;

        m_firstEmpty = std::min(m_firstEmpty, accessor.m_index);
    }

    void CullingBuffer::SetBounds(const uint32_t index, cr<Bounds> bounds)
    {
        m_buffer.centerX[index] = bounds.center.x;
        m_buffer.centerY[index] = bounds.center.y;
        m_buffer.centerZ[index] = bounds.center.z;
        m_buffer.extentsX[index] = bounds.extents.x;
        m_buffer.extentsY[index] = bounds.extents.y;
        m_buffer.extentsZ[index] = bounds.extents.z;
    }

    bool CullingBuffer::GetVisible(const uint32_t index)
    {
        return m_buffer.visible[index] != 0;
    }

    void CullingBuffer::Cull(cr<arr<Vec4, 6>> planes)
    {
        auto count = m_buffer.centerX.Size();
        auto threadCount = GetGR()->GetJobScheduler()->GetThreadCount();
        auto batchSize = ceil_div(count, threadCount);
        batchSize = ceil_div(batchSize, 4) * 4;
        batchSize = std::max(batchSize, 4u);

        // assert(m_cullJobId == 0);
        
        m_cullJobId = GetGR()->GetJobScheduler()->ScheduleFixedBatchSize(m_buffer.centerX.Size(), batchSize,
            [planes, this](const uint32_t start, const uint32_t end)
            {
                this->CullBatch(planes, start, end);
            });

        GetGR()->GetJobScheduler()->Wait(m_cullJobId);
        GetGR()->GetBatchRenderUnit()->StartEncodingCmds();
    }

    void CullingBuffer::CullBatch(cr<arr<Vec4, 6>> planes, uint32_t start, uint32_t end)
    {
        ZoneScoped;
        
        arr<SimdVec4, 6> plane;
        arr<SimdVec4, 6> planeSign;

        for (uint32_t i = 0; i < 6; ++i)
        {
            auto p = _mm_load_ps(&planes[i].x);
            plane[i] = SimdVec4(p);

            auto s = sign(p);
            planeSign[i] = SimdVec4(s);
            planeSign[i].w = _mm_set1_ps(1.0f);
        }
        
        auto resultP = _mm_set1_ps(1.0f);
        for (uint32_t i = start; i < end; i+=4)
        {
            SimdVec4 center = {
                _mm_load_ps(m_buffer.centerX.Data() + i),
                _mm_load_ps(m_buffer.centerY.Data() + i),
                _mm_load_ps(m_buffer.centerZ.Data() + i),
                _mm_set1_ps(0.0f)
            };
            SimdVec4 extents = {
                _mm_load_ps(m_buffer.extentsX.Data() + i),
                _mm_load_ps(m_buffer.extentsY.Data() + i),
                _mm_load_ps(m_buffer.extentsZ.Data() + i),
                _mm_set1_ps(1.0f)
            };

            for (uint32_t j = 0; j < 6; ++j)
            {
                auto offset = mul(extents, planeSign[j]);
                auto d0 = dot(plane[j], add(center, offset));
                auto d1 = dot(plane[j], sub(center, offset));
                
                auto cmp_d0 = _mm_cmpge_ps(d0, _mm_setzero_ps());
                auto cmp_d1 = _mm_cmpge_ps(d1, _mm_setzero_ps());

                resultP = _mm_and_ps(resultP, _mm_or_ps(cmp_d0, cmp_d1));
            }

            _mm_store_ps(m_buffer.visible.Data() + i, resultP);
        }
    }

    CullingBuffer::CullingSoA::CullingSoA()
    {
        centerX = sl<float>(1024);
        centerY = sl<float>(1024);
        centerZ = sl<float>(1024);
        extentsX = sl<float>(1024);
        extentsY = sl<float>(1024);
        extentsZ = sl<float>(1024);
        visible = sl<float>(1024);
    }
}
