#include "thread_pool.h"

namespace op
{
    ThreadPool::ThreadPool(const uint32_t threadCount)
    {
        m_threads = vec<ConsumerThread<func_ptr>*>(threadCount);
        for (uint32_t i = 0; i < threadCount; ++i)
        {
            auto thread = new ConsumerThread<func_ptr>(1024, ExecuteFunc);
            
            m_threads[i] = thread;
        }
    }

    ThreadPool::~ThreadPool()
    {
        Stop(true);
    }

    void ThreadPool::Wait()
    {
        for (auto thread : m_threads)
        {
            thread->Wait();
        }
    }

    inline void ThreadPool::Stop(const bool immediate)
    {
        if (m_threads.empty())
        {
            return;
        }
        
        for (auto thread : m_threads)
        {
            thread->Stop(immediate);
        }

        for (auto thread : m_threads)
        {
            thread->Join();

            func_ptr func;
            while (thread->Dequeue(func))
            {
                FunctionPool<void()>::Ins()->Free(func);
            }

            delete thread;
        }

        m_threads.clear();
    }

    void ThreadPool::ExecuteFunc(const func_ptr func)
    {
        (*func)();

        FunctionPool<void()>::Ins()->Free(func);
    }
}
