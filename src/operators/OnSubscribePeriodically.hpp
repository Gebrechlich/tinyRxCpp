#ifndef ONSUBSCRIBEPERIODICALLY_HPP
#define ONSUBSCRIBEPERIODICALLY_HPP
#include "OnSubscribeBase.hpp"
#include "../Scheduler.hpp"
#include <chrono>
#include <iostream>
#include <limits>

template<typename T, typename Rep, typename Period>
class OnSubscribePeriodically : public OnSubscribeBase<T>
{
public:
    using OnSubscribePtrType = std::shared_ptr<OnSubscribeBase<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    OnSubscribePeriodically(Scheduler::SchedulerRefType&& s,
                                  const std::chrono::duration<Rep, Period>&  delay,
                                  const std::chrono::duration<Rep, Period>&  period,
                                  size_t count = std::numeric_limits<size_t>::max()) :
        scheduler(std::move(s)), delay(delay), period(period), count(count)
    {
    }

    struct PeriodicallyAction : public Action0
    {
        PeriodicallyAction(ThisSubscriberType c) : child(c)
        {}

        virtual void operator()() override
        {
            child->onNext(count);
            ++count;
        }

        size_t count = 0;
        ThisSubscriberType child;
    };

    void operator()(const ThisSubscriberType& s) override
    {
        auto worker = std::move(scheduler->createWorker());
        auto ssubscription = worker->schedulePeriodically(std::make_shared<PeriodicallyAction>(s),delay, period, count);
        workers.push_back(std::move(worker));
        s->add(ssubscription);
    }

private:
    Scheduler::SchedulerRefType scheduler;
    std::vector<Scheduler::WorkerRefType> workers; //keep refrences to workers
    const std::chrono::duration<Rep, Period> delay;
    const std::chrono::duration<Rep, Period> period;
    size_t count;
};

#endif // ONSUBSCRIBEPERIODICALLY_HPP
