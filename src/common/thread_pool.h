#pragma once
#include <queue>
#include <thread>

#include "function_pool.h"
#include "math/vec.h"

namespace op
{
    class ThreadPool
    {
        using Task = std::function<void()>*;
        
    public:
        explicit ThreadPool(uint32_t numThreads);
        ~ThreadPool();
        ThreadPool(const ThreadPool& other) = delete;
        ThreadPool(ThreadPool&& other) noexcept = delete;
        ThreadPool& operator=(const ThreadPool& other) = delete;
        ThreadPool& operator=(ThreadPool&& other) noexcept = delete;

        template <typename F>
        void Run(F&& func, int32_t priority = 0);

    private:
        vec<std::thread> m_threads;
        std::priority_queue<std::pair<int32_t, Task>> m_tasks;
        std::mutex m_taskMutex;
        std::condition_variable m_taskCond;
        bool m_shutdown = false;

        void Run(Task task, int32_t priority = 0);
        void Worker();
    };

    template <typename F>
    void ThreadPool::Run(F&& func, int32_t priority)
    {
        auto task = FunctionPool<void()>::Ins()->Alloc(std::forward<F>(func));
        Run(task, priority);
    }
}
