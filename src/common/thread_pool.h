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
        void Run(F&& func);

    private:
        vec<std::thread> m_threads;
        std::queue<Task> m_tasks;
        std::mutex m_taskMutex;
        std::condition_variable m_taskCond;
        bool m_shutdown = false;

        void Run(Task task);
        void Worker();
    };

    template <typename F>
    void ThreadPool::Run(F&& func)
    {
        auto task = FunctionPool<void()>::Ins()->Alloc(std::forward<F>(func));
        Run(task);
    }
}
