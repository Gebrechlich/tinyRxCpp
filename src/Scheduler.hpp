#ifndef SCHEDULER
#define SCHEDULER
#include "Subscription.hpp"
#include "Functions.hpp"
#include <chrono>
#include <thread>
#include <atomic>
#include <limits>

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
                                               const std::chrono::duration<Rep, Period>&  period , size_t count) :
        ScheduledAction(std::move(act)), delay(delay), period(period), count(count)
    {}

    virtual void operator()()
    {
        std::this_thread::sleep_for(delay);
        while(!this->unsubscr.load() && count)
        {
            (*this->action)();
            std::this_thread::sleep_for(period);
            if(count != std::numeric_limits<int>::max())
            {
                --count;
            }
        }
    }
private:
    const std::chrono::duration<Rep, Period>&  delay;
    const std::chrono::duration<Rep, Period>&  period;
    size_t count;
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

        SubscriptionPtrType schedule(ActionRefType action)
        {
            auto scAction = std::make_shared<ScheduledAction>(action);
            auto internalSubscription = scheduleInteranal(scAction);

            if(internalSubscription == nullptr)
            {
                return scAction;
            }

            auto subscriptions = std::make_shared<SubscriptionsList>();
            subscriptions->add(scAction);
            subscriptions->add(internalSubscription);

            return subscriptions;
        }

        template<typename Rep, typename Period>
        SubscriptionPtrType schedulePeriodically(ActionRefType action, const std::chrono::duration<Rep, Period>&  delay,
                                          const std::chrono::duration<Rep, Period>&  period, size_t count = std::numeric_limits<size_t>::max())
        {
            auto scAction = std::make_shared<PeriodicScheduledAction<Rep, Period>>(action, delay, period, count);
            auto internalSubscription = scheduleInteranal(scAction);

            if(internalSubscription == nullptr)
            {
                return scAction;
            }

            auto subscriptions = std::make_shared<SubscriptionsList>();
            subscriptions->add(scAction);
            subscriptions->add(internalSubscription);

            return subscriptions;
        }
    protected:
        virtual SubscriptionPtrType scheduleInteranal(ActionRefType action) = 0;
    };

    using WorkerRefType = std::shared_ptr<Worker>;
    virtual WorkerRefType createWorker() = 0;
};

#endif // SCHEDULER

