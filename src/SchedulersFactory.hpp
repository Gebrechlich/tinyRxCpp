#ifndef SCHEDULERSFACTORY
#define SCHEDULERSFACTORY

#include "NewThreadScheduler.hpp"
#include <mutex>

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
private:
    SchedulersFactory() = default;
    ~SchedulersFactory() = default;
    SchedulersFactory(const SchedulersFactory&) = default;

    std::mutex lockMutex;
    Scheduler::SchedulerRefType newThreadInstance;
};

#endif // SCHEDULERSFACTORY

