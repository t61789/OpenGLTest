#pragma once
#include <functional>
#include <mutex>
#include <tracy/Tracy.hpp>

#include "const.h"
#include "common/function_pool.h"
#include "common/thread_pool.h"

namespace op
{
    using task_func = std::function<void()>;
    using parallel_task_func = std::function<void(uint32_t, uint32_t)>;
    using task_finished_func = std::function<void(size_t)>;

    class Job
    {
    public:
        Job() = default;
        ~Job();
        Job(const Job& other) = delete;
        Job(Job&& other) noexcept = delete;
        Job& operator=(const Job& other) = delete;
        Job& operator=(Job&& other) noexcept = delete;

        void WaitForStart();
        void WaitForStop();
        void AppendNext(crsp<Job> next);

        bool IsComplete() const { return m_completed; }
        void SetMinBatchSize(const uint32_t minBatchSize) { m_minBatchSize = minBatchSize; }
        
        template <typename TaskFunc>
        static sp<Job> CreateCommon(TaskFunc&& f);
        template <typename TaskFunc>
        static sp<Job> CreateParallel(uint32_t taskElemCount, TaskFunc&& f);

    private:
        uint32_t m_taskElemCount = 0;
        uint32_t m_minBatchSize = 32;
        size_t m_taskGroupId = 0;
        
        task_func* m_taskFunc = nullptr;
        parallel_task_func* m_parallelTaskFunc = nullptr;
        
        uint32_t m_exceptTaskNum = 0;
        uint32_t m_completeTaskNum = 0;
        uint32_t m_completed = false;

        vecsp<Job> m_next;
        
        std::mutex m_accessMutex;
        std::condition_variable m_startingSignal;
        std::condition_variable m_completeSignal;
        
        bool CompleteOnce();

        friend class JobScheduler;
    };
    
    class JobScheduler
    {
    public:
        JobScheduler();
        ~JobScheduler();
        JobScheduler(const JobScheduler& other) = delete;
        JobScheduler(JobScheduler&& other) noexcept = delete;
        JobScheduler& operator=(const JobScheduler& other) = delete;
        JobScheduler& operator=(JobScheduler&& other) noexcept = delete;

        void Schedule(crsp<Job> job);

    private:

        up<ThreadPool> m_threadPool;
        vec<std::pair<size_t, sp<Job>>> m_runningParallelTasks;
        std::mutex m_schedulerMutex;

        void ScheduleCommonJob(crsp<Job> job);
        void ScheduleParallelJob(crsp<Job> job);
        void JobComplete(crsp<Job> job);
    };

    template <typename CommonTaskFunc>
    sp<Job> Job::CreateCommon(CommonTaskFunc&& f)
    {
        auto job = msp<Job>();
        job->m_taskGroupId = std::hash<size_t>{}(reinterpret_cast<uintptr_t>(job.get()));
        job->m_taskFunc = FunctionPool<void()>::Ins()->Alloc(std::forward<CommonTaskFunc>(f));
        job->m_parallelTaskFunc = nullptr;

        return job;
    }

    template <typename ParallelTaskFunc>
    sp<Job> Job::CreateParallel(const uint32_t taskElemCount, ParallelTaskFunc&& f)
    {
        auto job = msp<Job>();
        job->m_taskElemCount = taskElemCount;
        job->m_taskGroupId = std::hash<size_t>{}(reinterpret_cast<uintptr_t>(job.get()));
        job->m_taskFunc = nullptr;
        job->m_parallelTaskFunc = FunctionPool<void(uint32_t, uint32_t)>::Ins()->Alloc(std::forward<ParallelTaskFunc>(f));

        return job;
    }
}
