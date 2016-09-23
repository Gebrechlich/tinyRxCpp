#ifndef SCHEDULER
#define SCHEDULER
#include "Subscription.h"
#include "Functions.hpp"
#include <chrono>
#include <thread>
#include <atomic>

class ScheduledAction : public Action0, public SubscriptionBase
{
public:
    virtual ~ScheduledAction()
    {}

    ScheduledAction(ActionRefType act) : action(std::move(act)), unsubscr(false)
    {}

    virtual void operator()() override
    {
        if(!unsubscr.load())
        {
            (*action)();
        }
    }

    bool isUnsubscribe()
    {
        return unsubscr.load();
    }

    void unsubscribe()
    {
        unsubscr.store(true);
    }
protected:
    ActionRefType action;
    std::atomic<bool> unsubscr;
};

template<typename Rep, typename Period>
class PeriodicScheduledAction : public ScheduledAction
{
public:
    PeriodicScheduledAction(ActionRefType act, const std::chrono::duration<Rep, Period>&  delay,
                                               const std::chrono::duration<Rep, Period>&  period) :
        ScheduledAction(std::move(act)), delay(delay), period(period)
    {}

    virtual void operator()()
    {
        std::this_thread::sleep_for(delay);
        while(!this->unsubscr.load())
        {
            (*this->action)();
            std::this_thread::sleep_for(period);
        }
    }
private:
    const std::chrono::duration<Rep, Period>&  delay;
    const std::chrono::duration<Rep, Period>&  period;
};


using ScheduledActionPrtType = std::shared_ptr<ScheduledAction>;

class Scheduler
{
public:
    virtual ~Scheduler() = default;
    using SchedulerRefType = std::shared_ptr<Scheduler>;

    class Worker
    {
    public:
        virtual ~Worker() = default;

        SharedSubscription schedule(ActionRefType action)
        {
            ScheduledActionPrtType scAction = std::make_shared<ScheduledAction>(action);
            scheduleInteranal(scAction);
            return SharedSubscription(scAction);
        }

        template<typename Rep, typename Period>
        SharedSubscription schedulePeriodically(ActionRefType action, const std::chrono::duration<Rep, Period>&  delay,
                                          const std::chrono::duration<Rep, Period>&  period)
        {
            ScheduledActionPrtType scAction = std::make_shared<PeriodicScheduledAction<Rep, Period>>(action, delay, period);
            scheduleInteranal(scAction);
            return SharedSubscription(scAction);
        }

        virtual SharedSubscription getSubscription() = 0;

    protected:
        virtual void scheduleInteranal(ActionRefType action) = 0;
    };

    using WorkerRefType = std::unique_ptr<Worker>;
    virtual WorkerRefType createWorker() = 0;
};

#endif // SCHEDULER

