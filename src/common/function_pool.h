#pragma once
#include <functional>

#include "const.h"
#include "utils.h"

namespace op
{
    template <typename Signature>
    class FunctionPool
    {
        using func_type = std::function<Signature>;
        using func_ptr = func_type*;
        
    public:
        static FunctionPool* Ins();
        
        FunctionPool();
        ~FunctionPool();
        FunctionPool(const FunctionPool& other) = delete;
        FunctionPool(FunctionPool&& other) noexcept = delete;
        FunctionPool& operator=(const FunctionPool& other) = delete;
        FunctionPool& operator=(FunctionPool&& other) noexcept = delete;

        template <typename Lambda>
        func_ptr Alloc(Lambda&& lambda);
        void Free(func_ptr func);

    private:
        std::mutex m_allocMutex;
        sl<func_ptr> m_functions;
    };

    template <typename Signature>
    FunctionPool<Signature>* FunctionPool<Signature>::Ins()
    {
        static FunctionPool instance;
        return &instance;
    }

    template <typename Signature>
    FunctionPool<Signature>::FunctionPool()
    {
        m_functions = sl<func_ptr>(1024);
    }

    template <typename Signature>
    FunctionPool<Signature>::~FunctionPool()
    {
        for (uint32_t i = 0; i < m_functions.Size(); ++i)
        {
            delete m_functions[i];
        }
    }

    template <typename Signature>
    template <typename Lambda>
    typename FunctionPool<Signature>::func_ptr FunctionPool<Signature>::Alloc(Lambda&& lambda)
    {
        std::lock_guard lock(m_allocMutex);
        func_ptr f = nullptr;
        if (!m_functions.Pop(f))
        {
            f = new func_type();
        }

        *f = std::forward<Lambda>(lambda);
        return f;
    }

    template <typename Signature>
    void FunctionPool<Signature>::Free(func_ptr func)
    {
        if (!func)
        {
            return;
        }
        
        std::lock_guard lock(m_allocMutex);
        if (m_functions.Size() == m_functions.Capacity())
        {
            m_functions.Reserve(m_functions.Capacity() * 2);
        }

        *func = nullptr;
        m_functions.Add(func);
    }
}
