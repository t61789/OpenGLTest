#include "job_scheduler.h"

#include <mutex>
#include <tracy/Tracy.hpp>

#include "common/function_pool.h"

namespace op
{
    JobScheduler::JobScheduler()
    {
        m_threadPool = std::make_unique<ThreadPool>(2);
    }

    JobScheduler::~JobScheduler()
    {
        m_threadPool->Stop(true);
    }

    void JobScheduler::Wait(size_t taskGroupId)
    {
        ZoneScopedC(TRACY_IDLE_COLOR);
        
        if (taskGroupId == 0)
        {
            return;
        }
        
        std::unique_lock lock(m_taskCompleteMutex);
        auto taskGroupPtr = find(m_runningParallelTasks, taskGroupId);
        if (!taskGroupPtr)
        {
            return;
        }
        
        auto taskGroup = *taskGroupPtr;
        taskGroup->completeSignal.wait(lock, [this, taskGroupId]
        {
            return !find(m_runningParallelTasks, taskGroupId);
        });
    }

    JobScheduler::ParallelTask::~ParallelTask()
    {
        FunctionPool<void(uint32_t, uint32_t)>::Ins()->Free(func);
    }

    void JobScheduler::DispatchTask(crsp<ParallelTask> task)
    {
        uint32_t batchSize;
        if (task->fixedBatchSize)
        {
            batchSize = task->minBatchSize;
        }
        else
        {
            batchSize = (task->count + m_threadPool->GetThreadCount() - 1) / m_threadPool->GetThreadCount();
            batchSize = std::min(batchSize, task->minBatchSize);
            assert(batchSize > 0);
        }

        task->exceptTaskNum = 0;
        for (uint32_t start = 0; start < task->count; start += batchSize)
        {
            task->exceptTaskNum++;
        }

        insert<std::shared_ptr<ParallelTask>>(m_runningParallelTasks, task->taskGroupId, task);
        
        for (uint32_t start = 0; start < task->count; start += batchSize)
        {
            auto end = std::min(start + batchSize, task->count);
            m_threadPool->Start([start, end, task, this]
            {
                (*task->func)(start, end);

                std::lock_guard lock(this->m_taskCompleteMutex);
                task->completeTaskNum++;
                if (task->completeTaskNum == task->exceptTaskNum)
                {
                    task->completeSignal.notify_all();
                    remove(m_runningParallelTasks, task->taskGroupId);
                }
            });
        }
    }
}
