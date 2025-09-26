#include "job_scheduler.h"

#include <mutex>
#include <tracy/Tracy.hpp>

#include "common/function_pool.h"

namespace op
{
    JobScheduler::Job::~Job()
    {
        std::lock_guard lock(m_accessMutex);
        
        FunctionPool<void()>::Ins()->Free(m_taskFunc);
        FunctionPool<void(uint32_t, uint32_t)>::Ins()->Free(m_parallelTaskFunc);
    }

    void JobScheduler::Job::WaitForStart()
    {
        ZoneScopedC(TRACY_IDLE_COLOR);

        std::unique_lock lock(m_accessMutex);
        
        m_startingSignal.wait(lock, [this]
        {
            return m_exceptTaskNum != 0;
        });
    }

    void JobScheduler::Job::WaitForStop()
    {
        ZoneScopedC(TRACY_IDLE_COLOR);

        assert(m_exceptTaskNum != 0 && "Job has not been scheduled");
        
        std::unique_lock lock(m_accessMutex);
        
        m_completeSignal.wait(lock, [this]
        {
            return m_completed;
        });
    }

    void JobScheduler::Job::SetNext(crsp<Job> next)
    {
        std::lock_guard lock(m_accessMutex);
        
        m_next = next;
    }

    bool JobScheduler::Job::CompleteOnce()
    {
        std::lock_guard lock(m_accessMutex);
        
        m_completeTaskNum++;
        if (m_completeTaskNum == m_exceptTaskNum)
        {
            m_completed = true;
            return true;
        }
        return false;
    }
    
    
    JobScheduler::JobScheduler()
    {
        m_threadPool = std::make_unique<ThreadPool>(2);
    }

    JobScheduler::~JobScheduler()
    {
        while (!m_runningParallelTasks.empty())
        {
            sp<Job> job = nullptr;
            
            {
                std::lock_guard lock(m_schedulerMutex);
                if (m_runningParallelTasks.empty())
                {
                    break;
                }
                job = m_runningParallelTasks[0].second;
            }
            
            job->WaitForStop();
        }
        
        m_threadPool->Stop(false);
    }

    void JobScheduler::Schedule(crsp<Job> job)
    {
        std::lock_guard lock0(m_schedulerMutex);
        std::lock_guard lock1(job->m_accessMutex);
        
        assert(!job->m_completed);
        assert(!find(m_runningParallelTasks, job->m_taskGroupId));
        assert(!(job->m_taskFunc == nullptr && job->m_parallelTaskFunc == nullptr));
        assert(!(job->m_taskFunc != nullptr && job->m_parallelTaskFunc != nullptr));
        
        if (job->m_taskFunc != nullptr)
        {
            ScheduleCommonJob(job);
        }
        else if (job->m_parallelTaskFunc != nullptr)
        {
            ScheduleParallelJob(job);
        }
        
        insert(m_runningParallelTasks, job->m_taskGroupId, job);

        job->m_startingSignal.notify_all();
    }

    void JobScheduler::ScheduleCommonJob(crsp<Job> job)
    {
        job->m_exceptTaskNum = 1;
        job->m_completeTaskNum = 0;
        
        m_threadPool->Start([job, this]
        {
            (*job->m_taskFunc)();

            if (job->CompleteOnce())
            {
                this->JobComplete(job);
            }
        });
    }

    void JobScheduler::ScheduleParallelJob(crsp<Job> job)
    {
        assert(job->count != 0);
        
        uint32_t batchSize;
        if (job->fixedBatchSize)
        {
            batchSize = job->minBatchSize;
        }
        else
        {
            batchSize = (job->count + m_threadPool->GetThreadCount() - 1) / m_threadPool->GetThreadCount();
            batchSize = std::min(batchSize, job->minBatchSize);
            assert(batchSize > 0);
        }

        job->m_exceptTaskNum = 0;
        for (uint32_t start = 0; start < job->count; start += batchSize)
        {
            job->m_exceptTaskNum++;
        }
        
        for (uint32_t start = 0; start < job->count; start += batchSize)
        {
            auto end = std::min(start + batchSize, job->count);
            m_threadPool->Start([start, end, job, this]
            {
                (*job->m_parallelTaskFunc)(start, end);

                if (job->CompleteOnce())
                {
                    this->JobComplete(job);
                }
            });
        }
    }
    
    void JobScheduler::JobComplete(crsp<Job> job)
    {
        if (job->m_next)
        {
            Schedule(job->m_next);
        }
        
        {
            std::lock_guard lock(this->m_schedulerMutex);
            remove(this->m_runningParallelTasks, job->m_taskGroupId);
        }

        job->m_completeSignal.notify_all();
    }
}
