#ifndef ONSUBSCRIBEPERIODICALLY_HPP
#define ONSUBSCRIBEPERIODICALLY_HPP
#include "OnSubscribeBase.hpp"
#include "../Scheduler.hpp"
#include <chrono>

template<typename T, typename Rep, typename Period>
class OnSubscribePeriodically : public OnSubscribeBase<T>
{
public:
    using OnSubscribePtrType = std::shared_ptr<OnSubscribeBase<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    OnSubscribePeriodically(Scheduler::SchedulerRefType&& s,
                                  const std::chrono::duration<Rep, Period>&  delay,
                                  const std::chrono::duration<Rep, Period>&  period) :
        scheduler(std::move(s)), delay(delay), period(period)
    {}

    void operator()(const ThisSubscriberType& s) override
    {
        subscriber = s;
        worker = std::move(scheduler->createWorker());
        auto subscription = worker->getSubscription();
        s->add(subscription);
        auto ssubscription = worker->schedulePeriodically(std::make_shared<Action0>([this](){
            this->subscriber->onNext(this->count);
            ++this->count;
        }),delay, period);
        s->add(ssubscription);
    }

private:
    int count = 0;
    Scheduler::SchedulerRefType scheduler;
    Scheduler::WorkerRefType worker;
    const std::chrono::duration<Rep, Period> delay;
    const std::chrono::duration<Rep, Period> period;
    ThisSubscriberType subscriber;
};

#endif // ONSUBSCRIBEPERIODICALLY_HPP
