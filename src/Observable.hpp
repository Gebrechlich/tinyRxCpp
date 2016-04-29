#ifndef OBSERVABLE_H
#define OBSERVABLE_H
#include "Functions.hpp"
#include "Subscriber.hpp"
#include "OperatorFilter.hpp"
#include "OperatorMap.hpp"
#include "OperatorDistinct.hpp"
#include "OperatorAll.hpp"
#include "OperatorExist.hpp"
#include "OperatorScan.hpp"
#include "OperatorLast.hpp"
#include "OperatorTake.hpp"
#include "FunctionsPtr.hpp"
#include "Scheduler.hpp"
#include "MTQueue.h"
#include "Util.hpp"
#include <memory>
#include <initializer_list>

template<typename T>
using SubscriberPtrType = std::shared_ptr<Subscriber<T>>;

template<typename R>
class OnSubscribeBase : public Action1<SubscriberPtrType<R>>
{
public:
    OnSubscribeBase() : Action1<SubscriberPtrType<R>>(){}

    OnSubscribeBase(typename Action1<SubscriberPtrType<R>>::ActionFp fp) :
        Action1<SubscriberPtrType<R>>(fp){}
};

template<typename A, typename B>
class LiftOnSubscribe : public OnSubscribeBase<A>
{
public:
    LiftOnSubscribe(std::shared_ptr<OnSubscribeBase<B>> parent, std::unique_ptr<Operator<B, A>> o) :
                    OnSubscribeBase<A>(), parentOnSubscribe(parent), op(std::move(o))
    {
    }

    void operator()(const SubscriberPtrType<A>& t) override
    {
        SubscriberPtrType<B> st = (*op)(t);
        (*parentOnSubscribe)(std::move(st));
    }

protected:
    std::shared_ptr<OnSubscribeBase<B>> parentOnSubscribe;
    std::unique_ptr<Operator<B, A>> op;
};

template<typename T>
class DeferOnSubscribe;

template<typename T>
class OperatorSubscribeOn;

template<typename T>
class OperatorObserveOn;

template<typename T>
class Observable
{
public:
    using ThisSubscriberType = Subscriber<T>;
    using ThisSubscriberPtrType = SubscriberPtrType<T>;
    using OnSubscribe = OnSubscribeBase<T>;
    using ThisOnSubscribePtrType = std::shared_ptr<OnSubscribe>;

    Observable() = default;

    Observable(ThisOnSubscribePtrType f) : onSubscribe(std::move(f))
    {}

    static Observable<T> create(ThisOnSubscribePtrType f)
    {
        return Observable(std::move(f));
    }

    static Observable<T> create(typename OnSubscribe::ActionFp action)
    {
        return Observable(ThisOnSubscribePtrType(std::make_shared<OnSubscribe>(action)));
    }

    static Observable<T> just(std::initializer_list<T> initList)
    {
        return Observable<T>::create([initList](const Observable<T>::
                                     ThisSubscriberPtrType& subscriber)
        {
            auto value = std::begin(initList);
            auto end = std::end(initList);
            while(value != end)
            {
                subscriber->onNext(*value);
                ++value;
            }
            subscriber->onComplete();
        });
    }

    static Observable<T> just(T value)
    {
        return Observable<T>::create([value](const Observable<T>::
                                     ThisSubscriberPtrType& subscriber)
        {
            subscriber->onNext(value);
            subscriber->onComplete();
        });
    }

    template<typename L, typename = typename std::enable_if<
                 std::is_array<L>::value || is_iterable<L>::value>::type>
    static Observable<T> from(L&& list)
    {
        return Observable<T>::create([list](const Observable<T>::
                                     ThisSubscriberPtrType& subscriber)
        {
            auto value = std::begin(list);
            auto end = std::end(list);
            while(value != end)
            {
                subscriber->onNext(*value);
                ++value;
            }
            subscriber->onComplete();
        });
    }

    static Observable<T> defer(Function0UniquePtr<Observable<T>>&& observableFactory);

    WeekSubscription subscribe(typename ThisSubscriberType::ThisOnNextFP next)
    {
        return Observable<T>::subscribe(std::move(createSubscriber(next,
                       typename ThisSubscriberType::ThisOnErrorFP(),
                       typename ThisSubscriberType::ThisOnCompleteFP())), this);
    }

    WeekSubscription subscribe(typename ThisSubscriberType::ThisOnNextFP next,
                   typename ThisSubscriberType::ThisOnCompleteFP complete)
    {
        return Observable<T>::subscribe(std::move(createSubscriber(next,
                   typename ThisSubscriberType::ThisOnErrorFP(), complete)), this);
    }

    WeekSubscription subscribe(typename ThisSubscriberType::ThisOnNextFP next,
                   typename ThisSubscriberType::ThisOnErrorFP error,
                   typename ThisSubscriberType::ThisOnCompleteFP complete)
    {
        return Observable<T>::subscribe(std::move(createSubscriber(next, error, complete)), this);
    }

    WeekSubscription subscribe(typename ThisSubscriberType::ThisOnNextFP next,
                   typename ThisSubscriberType::ThisOnErrorFP error)
    {
        return Observable<T>::subscribe(std::move(createSubscriber(next, error,
                   typename ThisSubscriberType::ThisOnCompleteFP())), this);
    }

    WeekSubscription subscribe(ThisSubscriberPtrType subscriber)
    {
        return Observable<T>::subscribe(subscriber, this);
    }

    Observable<T> subscribeOn(Scheduler::SchedulerRefType&& scheduler);

    Observable<T> observeOn(Scheduler::SchedulerRefType&& scheduler);

    template<typename R, typename P>
    Observable<P> lift(std::unique_ptr<Operator<R, P>>&& o, std::shared_ptr<OnSubscribeBase<R>> onSubs)
    {
        std::shared_ptr<OnSubscribeBase<P>> sp(std::make_shared<LiftOnSubscribe<P, R>>(onSubs, std::move(o)));
        return Observable<P>(sp);
    }

    Observable<T> filter(Predicat<T>&& pred)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorFilter<T>>(std::move(pred))),
                    this->onSubscribe);
    }

    Observable<T> distinct()
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorDistinct<T,T>>(asIs<T>)),
                    this->onSubscribe);
    }

    template<typename R>
    Observable<T> distinctTo(Function1_t<R, T>&& keyGenerator)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorDistinct<T,R>>(std::move(keyGenerator))),
                    this->onSubscribe);
    }

    template<typename R>
    Observable<T> distinct(R&& fun)
    {
        return distinctTo<typename std::result_of<R(const T&)>::type>(std::move(fun));
    }

    Observable<T> take(size_t index)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorTake<T>>(index)),
                    this->onSubscribe);
    }

    template<typename R>
    Observable<R> map(Function1UniquePtr<R, T>&& fun)
    {
        return lift(std::unique_ptr<Operator<T, R>>(make_unique<OperatorMap<T, R>>(std::move(fun))),
                    this->onSubscribe);
    }

    template<typename R>
    Observable<typename std::result_of<R(const T&)>::type> map(R&& fun)
    {
        return map<typename std::result_of<R(const T&)>::type>(std::move(fun));
    }

    Observable<bool> all(Predicat<T>&& pred)
    {
        return lift(std::unique_ptr<Operator<T, bool>>(make_unique<OperatorAll<T>>(std::move(pred))),
                    this->onSubscribe);
    }

    Observable<bool> exist(Predicat<T>&& pred)
    {
        return lift(std::unique_ptr<Operator<T, bool>>(make_unique<OperatorExist<T>>(std::move(pred))),
                    this->onSubscribe);
    }

    Observable<T> scan(Function2UniquePtr<T,T,T>&& accumulator)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorScan<T, T>>(std::move(accumulator))),
                    this->onSubscribe);
    }

    Observable<T> scan(Function2UniquePtr<T,T,T>&& accumulator, Function0UniquePtr<T>&& seed)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorScan<T, T>>(std::move(accumulator),
                                                                           std::move(seed))),
                    this->onSubscribe);
    }

    Observable<T> last()
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorLast<T>>()),
                    this->onSubscribe);
    }

    Observable<T> reduce(Function2UniquePtr<T,T,T>&& accumulator)
    {
        return scan(std::move(accumulator)).last();
    }

protected:
    template<typename B>
    static WeekSubscription subscribe(SubscriberPtrType<B> subscriber,
                          Observable<B>* observable)
    {
        std::weak_ptr<SubscriptionBase> ptr = subscriber;
        WeekSubscription subs(std::move(ptr));
        subscriber->onStart();
        (*observable->onSubscribe)(subscriber);
        return subs;
    }

private:
    ThisOnSubscribePtrType onSubscribe;

    ThisSubscriberPtrType createSubscriber(
            typename ThisSubscriberType::ThisOnNextFP next,
            typename ThisSubscriberType::ThisOnErrorFP error,
            typename ThisSubscriberType::ThisOnCompleteFP complete)
    {
        return ThisSubscriberPtrType(std::make_shared<ThisSubscriberType>(next, error, complete));
    }
};

template<typename T>
class DeferOnSubscribe : public OnSubscribeBase<T>
{
public:
    DeferOnSubscribe(Function0UniquePtr<Observable<T>>&& observFactory) :
        observableFactory(std::move(observFactory))
    {
    }

    void operator()(const SubscriberPtrType<T>& t) override
    {
        auto o = (*observableFactory)();
        o.subscribe(std::move(const_cast<SubscriberPtrType<T>&>(t)));
    }
private:
    Function0UniquePtr<Observable<T>> observableFactory;
};

template<typename T>
Observable<T> Observable<T>::defer(Function0UniquePtr<Observable<T> >&& observableFactory)
{
    return create(std::shared_ptr<Observable<T>::OnSubscribe>(
                       std::make_shared<DeferOnSubscribe<T>>(std::move(observableFactory))));
}

template<typename T>
class OperatorSubscribeOn : public OnSubscribeBase<T>
{
public:
    OperatorSubscribeOn(Observable<T>& source, Scheduler::SchedulerRefType&& s) :
        source(source), scheduler(std::move(s))
    {}

    class ThreadAction : public Action0
    {
    public:
        ThreadAction(Observable<T>& source, const SubscriberPtrType<T>& subscriber)
            : source(source), subscriber(subscriber)
        {}
        void operator()() override
        {
            source.subscribe(subscriber);
        }
    private:
        Observable<T>& source;
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
    Observable<T>& source;
    Scheduler::SchedulerRefType scheduler;
    Scheduler::WorkerRefType worker;
};

template<typename T>
Observable<T> Observable<T>::subscribeOn(Scheduler::SchedulerRefType&& scheduler)
{
    return create(std::shared_ptr<Observable<T>::OnSubscribe>(
                      std::make_shared<OperatorSubscribeOn<T>>(*this, std::move(scheduler))));
}

template<typename T>
class OperatorObserveOn : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    struct ObserveOnSubscriber : public CompositeSubscriber<T,T>
    {
        struct ThreadAction : public Action0
        {
            ThreadAction(const ThisSubscriberType& c, const std::shared_ptr<MTQueue<T>>& q,
                         std::shared_ptr<SubscriptionBase>&& s) : child(c), queue(q), subscription(std::move(s))
            {}

            void operator()() override
            {
                while(!subscription.isUnsubscribe())
                {
                    T v = (*queue).waitAndPop();
                    child->onNext(v);
                }
            }

            ThisSubscriberType child;
            std::shared_ptr<MTQueue<T>> queue;
            SharedSubscription subscription;
        };

        ObserveOnSubscriber(ThisSubscriberType p, Scheduler::SchedulerRefType&& s) :
            CompositeSubscriber<T,T>(p), scheduler(std::move(s))
        {
            queue = std::make_shared<MTQueue<T>>();
        }

        void onNext(const T& t) override
        {
            if(!this->isUnsubscribe())
            {
                (*queue).push(t);
            }
        }

        void onComplete() override
        {
        }

        void init()
        {
            worker = std::move(scheduler->createWorker());
            auto subscription = worker->getSubscription();
            this->child->add(subscription);
            this->addChildSubscriptionFromThis();
            worker->schedule(std::make_shared<ThreadAction>(this->child, queue, this->shared_from_this()));
        }

        Scheduler::SchedulerRefType scheduler;
        Scheduler::WorkerRefType worker;
        std::shared_ptr<MTQueue<T>> queue;
    };
public:
    OperatorObserveOn(Scheduler::SchedulerRefType&& s) : scheduler(std::move(s))
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<ObserveOnSubscriber>(t, std::move(scheduler));
        subs->init();
        return subs;
    }
private:
    Scheduler::SchedulerRefType scheduler;
};

template<typename T>
Observable<T> Observable<T>::observeOn(Scheduler::SchedulerRefType&& scheduler)
{
    return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorObserveOn<T>>(std::move(scheduler))),
                this->onSubscribe);
}
#endif // OBSERVABLE_H