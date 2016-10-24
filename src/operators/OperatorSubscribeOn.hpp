#ifndef OPERATORSUBSCRIBEON_HPP
#define OPERATORSUBSCRIBEON_HPP
#include "OnSubscribeBase.hpp"
#include "../Scheduler.hpp"

template<typename T>
class OperatorSubscribeOn : public OnSubscribeBase<T>
{
public:
    using OnSubscribePtrType = std::shared_ptr<OnSubscribeBase<T>>;

    OperatorSubscribeOn(OnSubscribePtrType source, Scheduler::SchedulerRefType&& s) :
        source(source), scheduler(std::move(s))
    {}

    class ThreadAction : public Action0
    {
    public:
        ThreadAction(OnSubscribePtrType source, const SubscriberPtrType<T>& subscriber)
            : source(source), subscriber(subscriber)
        {}
        void operator()() override
        {
            (*source)(subscriber);
        }
    private:
        OnSubscribePtrType source;
        SubscriberPtrType<T> subscriber;
    };

    void operator()(const SubscriberPtrType<T>& subscriber) override
    {
        worker = std::move(scheduler->createWorker());
        auto subscription = worker->getSubscription();
        subscriber->add(subscription);
        worker->schedule(std::unique_ptr<Action0>(make_unique<ThreadAction>(source, subscriber)));
    }
private:
    OnSubscribePtrType source;
    Scheduler::SchedulerRefType scheduler;
    Scheduler::WorkerRefType worker;
};

#endif // OPERATORSUBSCRIBEON_HPP
