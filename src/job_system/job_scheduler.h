#pragma once
#include <functional>
#include <mutex>
#include <boost/lockfree/queue.hpp>
#include <tracy/Tracy.hpp>

#include "const.h"
#include "common/consumer_thread.h"
#include "common/function_pool.h"
#include "common/thread_pool.h"

namespace op
{
    class JobScheduler
    {
        using parallel_task_func = std::function<void(uint32_t, uint32_t)>;
        using task_finished_func = std::function<void(size_t)>;
        
    public:
        JobScheduler();
        ~JobScheduler();
        JobScheduler(const JobScheduler& other) = delete;
        JobScheduler(JobScheduler&& other) noexcept = delete;
        JobScheduler& operator=(const JobScheduler& other) = delete;
        JobScheduler& operator=(JobScheduler&& other) noexcept = delete;

        uint32_t GetThreadCount() const { return m_threadPool->GetThreadCount(); }

        template <typename ParallelTaskFunc>
        size_t Schedule(uint32_t count, ParallelTaskFunc&& func, uint32_t minBatchSize=~0u);
        template <typename ParallelTaskFunc>
        size_t ScheduleFixedBatchSize(uint32_t count, uint32_t batchSize, ParallelTaskFunc&& func);
        void Wait(size_t taskGroupId);

    private:
        struct ParallelTask
        {
            size_t taskGroupId;
            uint32_t count;
            uint32_t minBatchSize;
            bool fixedBatchSize;
            parallel_task_func* func;
            
            uint32_t exceptTaskNum;
            uint32_t completeTaskNum;
            std::condition_variable completeSignal;

            ~ParallelTask();
        };

        up<ThreadPool> m_threadPool;
        vec<std::pair<size_t, sp<ParallelTask>>> m_runningParallelTasks;
        std::mutex m_taskCompleteMutex;
        
        void DispatchTask(crsp<ParallelTask> task);
    };

    template <typename ParallelTaskFunc>
    size_t JobScheduler::Schedule(const uint32_t count, ParallelTaskFunc&& func, const uint32_t minBatchSize)
    {
        ZoneScoped;
        
        auto task = mup<ParallelTask>();
        auto taskGroupId = std::hash<size_t>{}(reinterpret_cast<uintptr_t>(task.get()));
        task->taskGroupId = taskGroupId;
        task->count = count;
        task->minBatchSize = std::max(minBatchSize, 1u);
        task->fixedBatchSize = false;
        task->func = FunctionPool<void(uint32_t, uint32_t)>::Ins()->Alloc(std::forward<ParallelTaskFunc>(func));

        assert(task->func && count != 0);

        DispatchTask(std::move(task));
        
        return taskGroupId;
    }

    template <typename ParallelTaskFunc>
    size_t JobScheduler::ScheduleFixedBatchSize(const uint32_t count, const uint32_t batchSize, ParallelTaskFunc&& func)
    {
        ZoneScoped;
        
        auto task = mup<ParallelTask>();
        auto taskGroupId = std::hash<size_t>{}(reinterpret_cast<uintptr_t>(task.get()));
        task->taskGroupId = taskGroupId;
        task->count = count;
        task->minBatchSize = std::max(batchSize, 1u);
        task->fixedBatchSize = true;
        task->func = FunctionPool<void(uint32_t, uint32_t)>::Ins()->Alloc(std::forward<ParallelTaskFunc>(func));

        assert(task->func && count != 0);
        
        DispatchTask(std::move(task));

        return taskGroupId;
    }
}
