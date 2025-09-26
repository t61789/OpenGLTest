#pragma once
#include <functional>
#include <mutex>
#include <boost/lockfree/queue.hpp>
#include <tracy/Tracy.hpp>

#include "const.h"
#include "utils.h"

namespace op
{
    template <typename T>
    class ConsumerThread
    {
        static_assert(std::is_trivial_v<T>);
        
    public:
        template <typename ConsumeFunc>
        explicit ConsumerThread(uint32_t productCapacity, ConsumeFunc&& f);
        ~ConsumerThread();
        ConsumerThread(const ConsumerThread& other) = delete;
        ConsumerThread(ConsumerThread&& other) noexcept = delete;
        ConsumerThread& operator=(const ConsumerThread& other) = delete;
        ConsumerThread& operator=(ConsumerThread&& other) noexcept = delete;

        void Enqueue(const T& product);
        bool Dequeue(T& product);
        void Stop(bool immediate = false);
        void Wait();
        void Join();

    private:
        lock_free_queue<T> m_taskQueue;
        std::mutex m_consumeMutex;
        std::condition_variable m_hasTaskCond;
        std::condition_variable m_idleCond;
        std::thread m_thread;

        std::atomic<bool> m_stopFlag = false;
        std::atomic<bool> m_stopImmediate = false;
        std::atomic<bool> m_isIdle = false;

        template <class ConsumeFunc>
        void Thread(ConsumeFunc&& consumeFunc);
    };

    template <typename T>
    template <typename ConsumeFunc>
    ConsumerThread<T>::ConsumerThread(uint32_t productCapacity, ConsumeFunc&& f) : m_taskQueue(productCapacity)
    {
        m_thread = std::thread([this, f=std::move(f)]
        {
            this->Thread(f);
        });
    }

    template <typename T>
    ConsumerThread<T>::~ConsumerThread()
    {
        Stop(true);

        Join();
    }

    template <typename T>
    void ConsumerThread<T>::Enqueue(const T& product)
    {
        ZoneScoped;

        m_taskQueue.push(product);
        m_hasTaskCond.notify_one();
    }

    template <typename T>
    bool ConsumerThread<T>::Dequeue(T& product)
    {
        return m_taskQueue.pop(product);
    }

    template <typename T>
    void ConsumerThread<T>::Stop(const bool immediate)
    {
        std::lock_guard lock(m_consumeMutex);
        m_stopFlag = true;
        m_stopImmediate = immediate;
        m_hasTaskCond.notify_one();
    }

    template <typename T>
    void ConsumerThread<T>::Wait()
    {
        std::unique_lock lock(m_consumeMutex);
        m_idleCond.wait(lock, [this]
        {
            return this->m_isIdle.load();
        });
    }

    template <typename T>
    void ConsumerThread<T>::Join()
    {
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

    template <typename T>
    template <typename ConsumeFunc>
    void ConsumerThread<T>::Thread(ConsumeFunc&& consumeFunc)
    {
        auto retryCount = 0;
        while (true)
        {
            {
                std::lock_guard lock(m_consumeMutex);
                if (m_stopFlag && (m_stopImmediate || m_taskQueue.empty()))
                {
                    break;
                }
            }
            
            T product;

            if (m_taskQueue.pop(product))
            {
                consumeFunc(product);
                retryCount = 0;
            }
            else
            {
                retryCount++;
            }

            if (retryCount == 3)
            {
                retryCount = 0;
                
                std::unique_lock lock(m_consumeMutex);
                m_isIdle = true;
                m_idleCond.notify_all();
                m_hasTaskCond.wait(lock, [this]
                {
                    return !this->m_taskQueue.empty() || this->m_stopFlag;
                });
                m_isIdle = false;
            }
        }

        std::lock_guard lock(m_consumeMutex);
        m_isIdle = true;
        m_idleCond.notify_all();
    }
}
