#ifndef SCHEDULERSFACTORY
#define SCHEDULERSFACTORY

#include "schedulers/NewThreadScheduler.hpp"
#include "schedulers/ThreadPoolScheduler.hpp"
#include <mutex>

#define DEFAULT_THREAD_POOL_SIZE std::thread::hardware_concurrency() * 2

class SchedulersFactory
{
public:
    static SchedulersFactory& instance()
    {
        static SchedulersFactory inst;
        return inst;
    }

    Scheduler::SchedulerRefType newThread()
    {
        if(!newThreadInstance)
        {
            std::lock_guard<std::mutex> l(lockMutex);
            if(!newThreadInstance)
            {
                newThreadInstance = std::make_shared<NewThreadScheduler>();
            }
        }
        return newThreadInstance;
    }

    Scheduler::SchedulerRefType threadPoolScheduler(size_t poolSize = DEFAULT_THREAD_POOL_SIZE)
    {
        if(!threadPoolInstance)
        {
            std::lock_guard<std::mutex> l(lockMutex);
            if(!threadPoolInstance)
            {
                threadPoolInstance = std::make_shared<ThreadPoolScheduler>(poolSize);
            }
        }
        return threadPoolInstance;
    }
private:
    SchedulersFactory() = default;
    ~SchedulersFactory() = default;
    SchedulersFactory(const SchedulersFactory&) = default;

    std::mutex lockMutex;
    Scheduler::SchedulerRefType newThreadInstance;
    Scheduler::SchedulerRefType threadPoolInstance;
};

#endif // SCHEDULERSFACTORY

