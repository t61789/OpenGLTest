#include "culling_system.h"

#include <array>

#include <tracy/Tracy.hpp>

#include "render_context.h"
#include "bounds.h"
#include "game_resource.h"
#include "object.h"
#include "scene.h"
#include "objects/render_comp.h"

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
        m_input = sl<uint32_t>(1);
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
    }

    void CullingSystem::Cull()
    {
        ZoneScoped;
        
        // 把visibleRenderObjs扔了，剔除allRenderObjs生成一个新的
        GetRC()->visibleRenderObjs.clear();
    
        const auto& renderObjs = *GetRC()->allRenderObjs;
        auto planes = GetRC()->mainVPInfo->frustumPlanes.value();

        for (const auto& renderObj : renderObjs)
        {
            if (CullOnce(renderObj.lock()->GetWorldBounds(), planes))
            {
                GetRC()->visibleRenderObjs.push_back(renderObj.lock().get());
            }
        }
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

    
    void CullingBuffer::Accessor::Submit(cr<Bounds> bounds)
    {
        assert(m_enable);
        
        m_buffer->SetBounds(m_index, bounds);
    }

    CullingBuffer::~CullingBuffer()
    {
        for (auto accessor : m_accessors)
        {
            delete accessor;
        }
    }

    CullingBuffer::Accessor* CullingBuffer::Alloc()
    {
        for (auto searchIndex = m_firstEmpty; searchIndex <= m_accessors.size(); ++searchIndex)
        {
            if (searchIndex >= m_accessors.size())
            {
                for (uint32_t i = 0; i < 4; ++i)
                {
                    m_soa.Add();

                    auto accessor = new Accessor();
                    accessor->m_buffer = this;
                    accessor->m_enable = false;
                    accessor->m_index = m_accessors.size();
                    m_accessors.push_back(accessor);
                }
            }

            if (!m_accessors[searchIndex]->m_enable)
            {
                m_firstEmpty = searchIndex + 1;
                m_accessors[searchIndex]->m_enable = true;
                return m_accessors[searchIndex];
            }
        }

        throw std::runtime_error("CullingBuffer::Alloc: This could not happened");
    }

    void CullingBuffer::SetBounds(const uint32_t index, cr<Bounds> bounds)
    {
        m_soa.centerX[index] = bounds.center.x;
        m_soa.centerY[index] = bounds.center.y;
        m_soa.centerZ[index] = bounds.center.z;
        m_soa.extentsX[index] = bounds.extents.x;
        m_soa.extentsY[index] = bounds.extents.y;
        m_soa.extentsZ[index] = bounds.extents.z;
    }

    sp<Job> CullingBuffer::CreateCullJob(cr<arr<Vec4, 6>> planes)
    {
        assert(!m_cullJob || m_cullJob->IsComplete());
        
        // 每个simd命令为一组，需要除4
        auto job = Job::CreateParallel(m_soa.centerX.Size() / 4, [planes, this](const uint32_t start, const uint32_t end)
        {
            this->CullBatch(planes, start, end);
        });

        m_cullJob = job;

        return job;
    }

    void CullingBuffer::WaitForCull()
    {
        assert(m_cullJob);
        m_cullJob->WaitForStop();
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
        
        for (uint32_t j = start; j < end; j++)
        {
            auto i = j * 4;
            
            SimdVec4 center = {
                _mm_load_ps(m_soa.centerX.Data() + i),
                _mm_load_ps(m_soa.centerY.Data() + i),
                _mm_load_ps(m_soa.centerZ.Data() + i),
                _mm_set1_ps(0.0f)
            };
            SimdVec4 extents = {
                _mm_load_ps(m_soa.extentsX.Data() + i),
                _mm_load_ps(m_soa.extentsY.Data() + i),
                _mm_load_ps(m_soa.extentsZ.Data() + i),
                _mm_set1_ps(0.0f)
            };

            auto resultP = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps());
            for (uint32_t j = 0; j < 6; ++j)
            {
                auto offset = mul(extents, planeSign[j]);
                auto a = add(center, offset);
                auto s = sub(center, offset);
                a.w = s.w = _mm_set1_ps(1.0f);
                auto d0 = dot(plane[j], a);
                auto d1 = dot(plane[j], s);
                
                auto cmp_d0 = _mm_cmpge_ps(d0, _mm_setzero_ps());
                auto cmp_d1 = _mm_cmpge_ps(d1, _mm_setzero_ps());

                resultP = _mm_and_ps(resultP, _mm_or_ps(cmp_d0, cmp_d1));
            }

            _mm_store_ps(m_soa.visible.Data() + i, resultP);
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

    void CullingBuffer::CullingSoA::Add()
    {
        centerX.Add(0);
        centerY.Add(0);
        centerZ.Add(0);
        extentsX.Add(0);
        extentsY.Add(0);
        extentsZ.Add(0);
        visible.Add(1);
    }
}
