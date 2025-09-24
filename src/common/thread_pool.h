#pragma once
#include <cstdint>
#include <functional>
#include <mutex>

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

        void Start(const std::function<void()>& f);
        void Wait();
        void Stop(bool immediate = false);

    private:
        std::mutex m_enqueueMutex;
        std::condition_variable m_condition;
        std::condition_variable m_pendingCondition;
        std::vector<std::function<void()>> m_tasks;
        std::vector<std::thread> m_threads;
        bool m_stopFlag = false;
        bool m_stopImmediate = false;
        uint32_t m_remainingTasks = 0;

        void WorkerThread();
    };
}
