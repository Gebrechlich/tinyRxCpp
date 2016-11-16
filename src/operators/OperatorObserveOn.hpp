#ifndef OPERATOROBSERVEON_H
#define OPERATOROBSERVEON_H
#include "Operator.hpp"
#include "../utils/MTQueue.hpp"
#include "../Scheduler.hpp"
#include <atomic>
#include <exception>

struct SlowSubscriberException: public std::exception
{
  virtual const char* what() const noexcept
  {
    return "Processing data by subscriber is too slow.";
  }
};

template<typename T>
class OperatorObserveOn : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    struct ObserveOnSubscriber : public CompositeSubscriber<T,T>
    {
        struct State
        {
            State() : finished(false), ex(nullptr)
            {}
            volatile bool finished;
            std::exception_ptr ex;
        };

        struct ThreadAction : public Action0
        {
            ThreadAction(const ThisSubscriberType& c, const std::shared_ptr<MTQueue<T>>& q,
                         SubscriptionBase& s, State& st) : child(c), queue(q),
                subscription(s), state(st)
            {}

            void operator()() override
            {
                while(true)
                {
                    if(checkTerminateState())
                    {
                        return;
                    }
                    T v;
                    bool res = (*queue).waitForAndPop(v, std::chrono::seconds(TIME_TO_WAIT));
                    if(res)
                    {
                        child->onNext(v);
                    }
                }
            }

            bool checkTerminateState()
            {
                if(subscription.isUnsubscribe())
                {
                    if(!queue->empty())
                    {
                        queue->clear();
                    }
                    return true;
                }
                else if(state.finished)
                {
                    if(!queue->empty())
                    {
                        if(state.ex)
                        {
                            queue->clear();
                            child->onError(state.ex);
                            return true;
                        }
                        return false;
                    }
                    child->onComplete();
                    return true;
                }
                return false;
            }

            const int TIME_TO_WAIT = 2;
            ThisSubscriberType child;
            std::shared_ptr<MTQueue<T>> queue;
            SubscriptionBase& subscription;
            State& state;
        };

        ObserveOnSubscriber(ThisSubscriberType p,const Scheduler::SchedulerRefType& s, size_t bufferSize) :
            CompositeSubscriber<T,T>(p), scheduler(s), bufferSize(bufferSize)
        {
            queue = std::make_shared<MTQueue<T>>();
            queue->setLimit(bufferSize);
        }

        void onNext(const T& t) override
        {
            if(!this->isUnsubscribe() && !state.finished)
            {
                if(!(*queue).offer(t))
                {
                    throw SlowSubscriberException();
                }
            }
        }

        void onComplete() override
        {
            state.finished = true;
        }

        void onError(std::exception_ptr ex) override
        {
            state.finished = true;
            state.ex = ex;
        }

        void init()
        {
            worker = scheduler->createWorker();
            auto ssubscription = worker->schedule(std::make_shared<ThreadAction>(this->child, queue, *this, state));
            this->add(ssubscription);
            this->addChildSubscriptionFromThis();
        }

        Scheduler::SchedulerRefType scheduler;
        Scheduler::WorkerRefType worker;
        std::shared_ptr<MTQueue<T>> queue;
        State state;
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
