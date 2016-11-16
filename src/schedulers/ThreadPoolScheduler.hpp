#ifndef THREADPOOLSCHEDULER_HPP
#define THREADPOOLSCHEDULER_HPP
#include "../Scheduler.hpp"
#include "../utils/Util.hpp"
#include "../utils/ThreadPoolExecutor.hpp"
#include <thread>

class ThreadPoolScheduler : public Scheduler
{
protected:
    class ThreadPoolWorker;
public:
    ThreadPoolScheduler(size_t poolSize)
    {
        pool = std::make_shared<ThreadPoolWorker>(poolSize);
    }

    WorkerRefType createWorker() override
    {
        return pool;
    }

protected:
    std::shared_ptr<ThreadPoolWorker> pool;

    class ThreadPoolWorker : public Scheduler::Worker
    {
    public:
        ThreadPoolWorker(size_t poolSize) : executor(poolSize)
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


#endif // THREADPOOLSCHEDULER_HPP
