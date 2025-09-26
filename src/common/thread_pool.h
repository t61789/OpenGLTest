#pragma once
#include <cstdint>
#include <functional>
#include <mutex>

#include "consumer_thread.h"
#include "function_pool.h"

namespace op
{
    class ThreadPool
    {
    public:
        explicit ThreadPool(uint32_t threadCount);
        ~ThreadPool();
        ThreadPool(const ThreadPool& other) = delete;
        ThreadPool(ThreadPool&& other) noexcept = delete;
        ThreadPool& operator=(const ThreadPool& other) = delete;
        ThreadPool& operator=(ThreadPool&& other) noexcept = delete;

        uint32_t GetThreadCount() const { return m_threads.size(); }
        
        template <typename Func>
        void Start(Func&& f);
        void Wait();
        void Stop(bool immediate);

    private:
        using func_ptr = std::function<void()>*;
        
        vec<ConsumerThread<func_ptr>*> m_threads;
        uint32_t m_preUsedThread = 0;

        void ExecuteFunc(func_ptr func);
    };

    template <typename Func>
    void ThreadPool::Start(Func&& f)
    {
        m_threads[m_preUsedThread]->Enqueue(FunctionPool<void()>::Ins()->Alloc(f));

        m_preUsedThread = (m_preUsedThread + 1) % m_threads.size();
    }
}
