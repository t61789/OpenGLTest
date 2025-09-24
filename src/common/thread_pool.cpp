#include "thread_pool.h"

namespace op
{
    ThreadPool::ThreadPool(const uint32_t threadCount)
    {
        m_threads.reserve(threadCount);
        for (uint32_t i = 0; i < threadCount; ++i)
        {
            m_threads.emplace_back(&ThreadPool::WorkerThread, this);
            m_remainingTasks++;
        }
    }

    ThreadPool::~ThreadPool()
    {
        m_stopFlag = true;
        m_condition.notify_all();

        for (auto& thread : m_threads)
        {
            thread.join();
        }
    }

    void ThreadPool::Start(const std::function<void()>& f)
    {
        std::lock_guard lock(m_enqueueMutex);
        m_tasks.emplace_back(f);
        
        m_condition.notify_one();
    }

    void ThreadPool::Wait()
    {
        std::unique_lock lock(m_enqueueMutex);

        if (m_remainingTasks == m_threads.size() && (m_tasks.empty() || m_stopFlag))
        {
            return;
        }

        m_pendingCondition.wait(lock, [this]
        {
            return this->m_remainingTasks == this->m_threads.size() && (this->m_tasks.empty() || this->m_stopFlag);
        });
    }

    void ThreadPool::Stop(const bool immediate)
    {
        m_stopFlag = true;
        m_stopImmediate = immediate;
        m_condition.notify_all();
    }

    void ThreadPool::WorkerThread()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock lock(m_enqueueMutex);

                if (m_tasks.empty())
                {
                    if (m_remainingTasks == m_threads.size())
                    {
                        m_pendingCondition.notify_all();
                    }
                    
                    m_condition.wait(lock, [this]
                    {
                        return !this->m_tasks.empty() || this->m_stopFlag;
                    });
                }

                if (!m_tasks.empty() && !m_stopImmediate)
                {
                    task = std::move(m_tasks.back());
                    m_tasks.pop_back();
                    
                    m_remainingTasks--;
                }
                else if (m_stopFlag)
                {
                    break;
                }
            }

            task();
            
            std::unique_lock lock(m_enqueueMutex);
            m_remainingTasks++;
        }

        m_pendingCondition.notify_all();
    }
}
