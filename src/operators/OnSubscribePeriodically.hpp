#ifndef ONSUBSCRIBEPERIODICALLY_HPP
#define ONSUBSCRIBEPERIODICALLY_HPP
#include "OnSubscribeBase.hpp"
#include "../Scheduler.hpp"
#include <chrono>
#include <iostream>
#include <mutex>
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
         worker = std::move(scheduler->createWorker());
    }

    struct PeriodicallyAction : public Action0
    {
        PeriodicallyAction(ThisSubscriberType c, std::mutex& m) : child(c), mt(m)
        {}

        virtual void operator()() override
        {
            mt.lock();
            child->onNext(count);
            mt.unlock();
            ++count;
        }

        size_t count = 0;
        ThisSubscriberType child;
        std::mutex& mt;
    };

    void operator()(const ThisSubscriberType& s) override
    {
        auto ssubscription = worker->schedulePeriodically(std::make_shared<PeriodicallyAction>(s, mt),delay, period, count);
        s->add(ssubscription);
    }

private:
    Scheduler::SchedulerRefType scheduler;
    Scheduler::WorkerRefType worker;
    const std::chrono::duration<Rep, Period> delay;
    const std::chrono::duration<Rep, Period> period;
    std::mutex mt;
    size_t count;
};

#endif // ONSUBSCRIBEPERIODICALLY_HPP
