#pragma once
#include <cstdint>

namespace op
{
    class IMemoryBlock
    {
    public:
        IMemoryBlock() = default;
        virtual ~IMemoryBlock() = default;
        IMemoryBlock(const IMemoryBlock& other) = delete;
        IMemoryBlock(IMemoryBlock&& other) noexcept = delete;
        IMemoryBlock& operator=(const IMemoryBlock& other) = delete;
        IMemoryBlock& operator=(IMemoryBlock&& other) noexcept = delete;

        virtual void Resize(uint32_t sizeB) = 0;
        virtual bool SetData(uint32_t offsetB, uint32_t sizeB, const void* data) = 0;
        virtual bool GetData(uint32_t offsetB, uint32_t sizeB, void* data) = 0;
        virtual uint32_t Size() const = 0;
    };
}
