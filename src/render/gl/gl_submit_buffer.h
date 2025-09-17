#pragma once
#include "const.h"
#include "common/i_memory_block.h"

namespace op
{
    class GlBuffer;
    
    class GlSubmitBuffer final : public IMemoryBlock
    {
    public:
        explicit GlSubmitBuffer(uint32_t type, uint32_t usage, uint32_t sizeB, uint32_t slot = ~0u);
        ~GlSubmitBuffer() override;
        GlSubmitBuffer(const GlSubmitBuffer& other) = delete;
        GlSubmitBuffer(GlSubmitBuffer&& other) noexcept = delete;
        GlSubmitBuffer& operator=(const GlSubmitBuffer& other) = delete;
        GlSubmitBuffer& operator=(GlSubmitBuffer&& other) noexcept = delete;

        void Bind();
        void BindBase();
        void Resize(uint32_t sizeB) override;
        bool SetData(uint32_t offsetB, uint32_t sizeB, const void* data) override;
        bool GetData(uint32_t offsetB, uint32_t sizeB, void* data) override;
        void Submit();

        crsp<GlBuffer> GetGlBuffer() const { return m_glBuffer;}
        uint32_t Size() const override { return m_sizeB;}

    private:
        struct BlockInfo
        {
            uint32_t offsetB;
            uint32_t sizeB;
        };
        
        uint32_t m_sizeB;
        uint32_t m_usage;
        
        vec<BlockInfo> m_dirtyBlocks;

        uint8_t* m_data;
        sp<GlBuffer> m_glBuffer;
        uint32_t m_slot;
    };
}
