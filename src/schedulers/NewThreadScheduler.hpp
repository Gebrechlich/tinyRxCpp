#ifndef NEWTHREADSCHEDULER
#define NEWTHREADSCHEDULER

#include "../Scheduler.hpp"
#include "../utils/Util.hpp"
#include "../utils/ThreadPoolExecutor.hpp"
#include <thread>

class NewThreadWorker;

class NewThreadScheduler : public Scheduler
{
public:
    WorkerRefType createWorker() override
    {
        return std::move(make_unique<NewThreadWorker>());
    }

protected:
    class NewThreadWorker : public Scheduler::Worker
    {
        struct NewThreadSubscription;
    public:
        NewThreadWorker() : executor(1), subscription(std::make_shared<NewThreadSubscription>(executor))
        {}

        NewThreadWorker(NewThreadWorker&&) = default;
        NewThreadWorker& operator = (NewThreadWorker&&) = default;

        NewThreadWorker(const NewThreadWorker&) = delete;
        NewThreadWorker& operator = (const NewThreadWorker&) = delete;

        void scheduleInteranal(ActionRefType action) override
        {
             executor.submit(action);
        }

        virtual SharedSubscription getSubscription() override
        {
            return subscription;
        }
    private:

        struct NewThreadSubscription : public SubscriptionBase
        {
            NewThreadSubscription(ThreadPoolExecutor& e) : executor(e), isUnsub(false)
            {}

            bool isUnsubscribe() override
            {
                if(isUnsub.load())
                {
                    executor.shutdown();
                }
                return isUnsub.load();
            }

            void unsubscribe() override
            {
                isUnsub.store(true);
            }

            ThreadPoolExecutor& executor;
            std::atomic<bool> isUnsub;
        };

        ThreadPoolExecutor executor;
        SharedSubscription subscription;
    };
};

#endif // NEWTHREADSCHEDULER

