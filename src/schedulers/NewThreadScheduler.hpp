#ifndef NEWTHREADSCHEDULER
#define NEWTHREADSCHEDULER

#include "Scheduler.hpp"
#include "Util.hpp"
#include "ThreadPoolExecutor.hpp"
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
        struct ActionAdapter;
    public:
        NewThreadWorker() : executor(1), subscription(std::make_shared<NewThreadSubscription>(executor))
        {}

        NewThreadWorker(NewThreadWorker&&) = default;
        NewThreadWorker& operator = (NewThreadWorker&&) = default;

        NewThreadWorker(const NewThreadWorker&) = delete;
        NewThreadWorker& operator = (const NewThreadWorker&) = delete;

        void schedule(ActionRefType action) override
        {
             executor.submit(ActionAdapter(action));
        }

        virtual SharedSubscription getSubscription() override
        {
            return subscription;
        }
    private:
        struct ActionAdapter
        {
            ActionAdapter(ActionRefType a) : action(a)
            {
            }
            void operator()()
            {
                if(action)
                {
                    (*action)();
                }
            }
            ActionRefType action;
        };
        struct NewThreadSubscription : public SubscriptionBase
        {
            NewThreadSubscription(ThreadPoolExecutor<ActionAdapter>& e) : executor(e), isUnsub(false)
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

            ThreadPoolExecutor<ActionAdapter>& executor;
            std::atomic<bool> isUnsub;
        };
        ThreadPoolExecutor<ActionAdapter> executor;
        SharedSubscription subscription;
    };
};

#endif // NEWTHREADSCHEDULER

