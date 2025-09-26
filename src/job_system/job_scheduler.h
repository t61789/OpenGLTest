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
        using task_func = std::function<void()>;
        using parallel_task_func = std::function<void(uint32_t, uint32_t)>;
        using task_finished_func = std::function<void(size_t)>;
        
    public:
        class Job
        {
        public:
            uint32_t count = 0;
            uint32_t minBatchSize = 1;
            bool fixedBatchSize = false;

            Job() = default;
            ~Job();
            Job(const Job& other) = delete;
            Job(Job&& other) noexcept = delete;
            Job& operator=(const Job& other) = delete;
            Job& operator=(Job&& other) noexcept = delete;

            void WaitForStart();
            void WaitForStop();
            void SetNext(crsp<Job> next);
            
            template <typename TaskFunc>
            static sp<Job> CreateCommon(TaskFunc&& f);
            template <typename TaskFunc>
            static sp<Job> CreateParallel(uint32_t count, TaskFunc&& f);
            
        private:
            size_t m_taskGroupId;
            
            task_func* m_taskFunc;
            parallel_task_func* m_parallelTaskFunc;
            
            uint32_t m_exceptTaskNum = 0;
            uint32_t m_completeTaskNum;
            uint32_t m_completed = false;
            std::mutex m_accessMutex;
            std::condition_variable m_startingSignal;
            std::condition_variable m_completeSignal;

            sp<Job> m_next;
            
            bool CompleteOnce();

            friend class JobScheduler;
        };
        
        JobScheduler();
        ~JobScheduler();
        JobScheduler(const JobScheduler& other) = delete;
        JobScheduler(JobScheduler&& other) noexcept = delete;
        JobScheduler& operator=(const JobScheduler& other) = delete;
        JobScheduler& operator=(JobScheduler&& other) noexcept = delete;

        uint32_t GetThreadCount() const { return m_threadPool->GetThreadCount(); }

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
    sp<JobScheduler::Job> JobScheduler::Job::CreateCommon(CommonTaskFunc&& f)
    {
        auto job = msp<Job>();
        job->m_taskGroupId = std::hash<size_t>{}(reinterpret_cast<uintptr_t>(job.get()));
        job->m_taskFunc = FunctionPool<void()>::Ins()->Alloc(std::forward<CommonTaskFunc>(f));
        job->m_parallelTaskFunc = nullptr;
        job->m_next = nullptr;

        return job;
    }

    template <typename ParallelTaskFunc>
    sp<JobScheduler::Job> JobScheduler::Job::CreateParallel(uint32_t count, ParallelTaskFunc&& f)
    {
        auto job = msp<Job>();
        job->count = count;
        job->m_taskGroupId = std::hash<size_t>{}(reinterpret_cast<uintptr_t>(job.get()));
        job->m_taskFunc = nullptr;
        job->m_parallelTaskFunc = FunctionPool<void(uint32_t, uint32_t)>::Ins()->Alloc(std::forward<ParallelTaskFunc>(f));
        job->m_next = nullptr;

        return job;
    }
}
