#ifndef NEWTHREADSCHEDULER
#define NEWTHREADSCHEDULER
#include "../Scheduler.hpp"
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
        ~NewThreadWorker()
        {
            if(workThread.joinable())
            {
                workThread.detach();
            }
        }
        SubscriptionPtrType scheduleInteranal(ActionRefType action) override
        {
            realAction = action;
            workThread = std::thread(&NewThreadWorker::run, this);
            //workThread.detach();
            return nullptr;
        }
    private:
        virtual void run()
        {
            if(realAction)
            {
                (*realAction)();
            }
        }
        std::thread workThread;
        ActionRefType realAction;
    };
};

#endif // NEWTHREADSCHEDULER

