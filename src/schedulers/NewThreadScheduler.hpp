#ifndef NEWTHREADSCHEDULER
#define NEWTHREADSCHEDULER
#include "../Scheduler.hpp"
#include "../utils/ThreadPoolExecutor.hpp"
#include <thread>

class NewThreadScheduler : public Scheduler
{
public:
    NewThreadScheduler()
    {}

    WorkerRefType createWorker() override
    {
        return std::make_shared<NewThreadWorker>();
    }
protected:
    class NewThreadWorker : public Scheduler::Worker
    {
    public:
        NewThreadWorker() : executor(1)
        {}

        SubscriptionPtrType scheduleInteranal(ActionRefType action) override
        {
            executor.submit(action);
            return nullptr;
        }
    private:
        ThreadPoolExecutor executor;
    };
};

#endif // NEWTHREADSCHEDULER

