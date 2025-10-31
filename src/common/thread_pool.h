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

        static void ExecuteFunc(func_ptr func);
    };

    template <typename Func>
    void ThreadPool::Start(Func&& f)
    {
        uint32_t minTaskCount = ~0u;
        auto minTaskThread = 0;
        for (uint32_t i = 0; i < m_threads.size(); ++i)
        {
            auto taskCount = m_threads[i]->GetTaskCount();
            if (taskCount < minTaskCount)
            {
                minTaskCount = taskCount;
                minTaskThread = i;
            }
        }
        m_threads[minTaskThread]->Enqueue(FunctionPool<void()>::Ins()->Alloc(f));
    }
}
