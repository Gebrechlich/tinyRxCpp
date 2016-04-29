#ifndef SCHEDULER
#define SCHEDULER
#include "Subscription.h"
#include "Functions.hpp"

class Scheduler
{
public:
    virtual ~Scheduler() = default;
    using SchedulerRefType = std::shared_ptr<Scheduler>;

    class Worker
    {
    public:
        using ActionRefType = std::shared_ptr<Action0>;
        virtual ~Worker() = default;
        virtual void schedule(ActionRefType action) = 0;
        virtual SharedSubscription getSubscription() = 0;
    };

    using WorkerRefType = std::unique_ptr<Worker>;
    virtual WorkerRefType createWorker() = 0;
};

#endif // SCHEDULER

