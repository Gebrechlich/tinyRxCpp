#ifndef NEWTHREADSCHEDULER
#define NEWTHREADSCHEDULER
#include "Scheduler.hpp"
#include "Util.hpp"
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
        NewThreadWorker() : subscription(std::make_shared<NewThreadSubscription>())
        {}

        ~NewThreadWorker()
        {
            if(!subscription.isUnsubscribe() && worker.joinable())
            {
                worker.join();
            }
            else if(worker.joinable())
            {
                worker.detach();
            }
        }
        NewThreadWorker(NewThreadWorker&&) = default;
        NewThreadWorker& operator = (NewThreadWorker&&) = default;

        NewThreadWorker(const NewThreadWorker&) = delete;
        NewThreadWorker& operator = (const NewThreadWorker&) = delete;

        void schedule(ActionRefType action) override
        {
            worker = std::thread([action]() {
                (*action)();
            });
        }
        virtual SharedSubscription getSubscription() override
        {
            return subscription;
        }
    private:
        struct NewThreadSubscription : public SubscriptionBase
        {
            bool isUnsubscribe() override
            {
                return isUnsub;
            }

            void unsubscribe() override
            {
                isUnsub = true;
            }

            volatile bool isUnsub = false;
        };
        std::thread worker;
        SharedSubscription subscription;
    };
};

#endif // NEWTHREADSCHEDULER

