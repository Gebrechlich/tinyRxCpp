#ifndef OPERATOROBSERVEON_H
#define OPERATOROBSERVEON_H
#include "Operator.hpp"
#include "../utils/MTQueue.hpp"
#include "../Scheduler.hpp"
#include <atomic>
#include "../exceptions/TRExceptions.hpp"

template<typename T>
class OperatorObserveOn : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    struct ObserveOnSubscriber : public CompositeSubscriber<T,T>
    {
        struct State
        {
            State(SubscriptionBase* subscription) :
                finished(false), currentValuesCount(0), ex(nullptr), subscription(subscription)
            {}
            std::atomic_bool finished;
            std::atomic_size_t currentValuesCount;
            std::exception_ptr ex;
            SubscriptionBase* subscription;
            std::mutex locker;
            MTQueue<T> queue;
        };

        using StateRefType = std::shared_ptr<State>;

        struct ThreadAction : public Action0
        {
            ThreadAction(const ThisSubscriberType& c, StateRefType st)
                : child(c), state(st)
            {}

            void operator()() override
            {
                while(true)
                {
                    bool done = state->finished.load();
                    bool empty = state->queue.empty();
                    size_t valuesCount = state->currentValuesCount.load();

                    bool terminate = checkTerminateState(done, empty, valuesCount, state->locker);

                    if(terminate || state->queue.empty())
                    {
                        return;
                    }

                    T v;
                    bool res = (state->queue).tryPop(v);
                    if(res)
                    {
                        child->onNext(v);
                        --state->currentValuesCount;
                    }
                }
            }

            bool checkTerminateState(bool isDone, bool isEmpty, size_t valuesCount, std::mutex& lock)
            {
                if(state->subscription->isUnsubscribe())
                {
                    if(!isEmpty)
                    {
                        state->queue.clear();
                    }
                    return true;
                }
                else if(isDone)
                {
                    if(!isEmpty)
                    {
                        std::unique_lock<std::mutex> locker(lock);
                        if(state->ex)
                        {
                            state->queue.clear();
                            child->onError(state->ex);
                            state->ex = nullptr;
                            state->subscription->unsubscribe();
                            return true;
                        }
                        return false;
                    } else if(valuesCount == 0)
                    {
                        std::unique_lock<std::mutex> locker(lock);
                        if(!state->subscription->isUnsubscribe())
                        {
                            child->onComplete();
                            state->subscription->unsubscribe();
                        }
                        return true;
                    }
                }
                return false;
            }

            ThisSubscriberType child;
            StateRefType state;
        };

        ObserveOnSubscriber(ThisSubscriberType p,const Scheduler::SchedulerRefType& s, size_t bufferSize) :
            CompositeSubscriber<T,T>(p), scheduler(s), bufferSize(bufferSize)
        {}

        void onNext(const T& t) override
        {
            if(!this->isUnsubscribe() && !state->finished.load())
            {
                if(!state->queue.offer(std::move(t)))
                {
                    throw SlowSubscriberException();
                }
                ++state->currentValuesCount;
                auto ssubscription = worker->schedule(std::make_shared<ThreadAction>(this->child, state));
                this->add(ssubscription);
            }
        }

        void onComplete() override
        {
            if(!state->finished.load())
            {
                state->finished.store(true);
            }
        }

        void onError(std::exception_ptr ex) override
        {
            if(state->finished.load())
            {
                return;
            }

            state->finished.store(true);
            std::unique_lock<std::mutex> guard(state->locker);
            state->ex = ex;
        }

        void init()
        {
            worker = scheduler->createWorker();
            state = std::make_shared<State>(this);
            state->queue.setLimit(bufferSize);
            this->addChildSubscriptionFromThis();
        }

        Scheduler::SchedulerRefType scheduler;
        Scheduler::WorkerRefType worker;
        StateRefType state;
        size_t bufferSize;
    };

public:
    OperatorObserveOn(const Scheduler::SchedulerRefType& scheduler,size_t bufferSize) :
        scheduler(scheduler), bufferSize(bufferSize)
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<ObserveOnSubscriber>(t, scheduler, bufferSize);
        subs->init();
        return subs;
    }
private:
    Scheduler::SchedulerRefType scheduler;
    size_t bufferSize;
};

#endif // OPERATOROBSERVEON_H
