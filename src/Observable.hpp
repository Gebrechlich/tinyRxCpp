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
#include "OperatorObserveOn.hpp"
#include "OperatorToMap.hpp"
#include "FunctionsPtr.hpp"
#include "Scheduler.hpp"
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

template<typename T, typename R>
class OnSubscribeConcatMap;

template<typename T>
class Observable
{
public:
    using ThisSubscriberType = Subscriber<T>;
    using ThisSubscriberPtrType = SubscriberPtrType<T>;
    using OnSubscribe = OnSubscribeBase<T>;
    using ThisOnSubscribePtrType = std::shared_ptr<OnSubscribe>;

    Observable() = default;
    ~Observable() = default;

    Observable(ThisOnSubscribePtrType f) : onSubscribe(std::move(f))
    {
    }

    static Observable<T> create(ThisOnSubscribePtrType f)
    {
        return Observable(std::move(f));
    }

    static Observable<T> create(typename OnSubscribe::ActionFp action)
    {
        return Observable(ThisOnSubscribePtrType(std::make_shared<OnSubscribe>(action)));
    }

    static Observable<T> just(std::vector<T> initList)
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

    Observable<T> observeOn(Scheduler::SchedulerRefType&& scheduler)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorObserveOn<T>>(std::move(scheduler))),
                    this->onSubscribe);
    }

    template<typename R, typename P>
    Observable<P> lift(std::unique_ptr<Operator<R, P>>&& o, std::shared_ptr<OnSubscribeBase<R>> onSubs)
    {
        std::shared_ptr<OnSubscribeBase<P>> sp(std::make_shared<LiftOnSubscribe<P, R>>(onSubs, std::move(o)));
        return Observable<P>(sp);
    }

    template<typename R, typename P>
    Observable<P> lift(std::unique_ptr<Operator<R, P>>&& o)
    {
        std::shared_ptr<OnSubscribeBase<P>> sp(std::make_shared<LiftOnSubscribe<P, R>>(this->onSubscribe, std::move(o)));
        return Observable<P>(sp);
    }

    Observable<T> filter(Predicat<T>&& pred)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorFilter<T>>(std::move(pred))));
    }

    Observable<T> distinct()
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorDistinct<T,T>>(asIs<T>)));
    }

    template<typename R>
    Observable<T> distinct(Function1_t<R, T>&& keyGenerator)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorDistinct<T,R>>(std::move(keyGenerator))));
    }

    template<typename R>
    Observable<T> distinct(R&& fun)
    {
        return distinct<typename std::result_of<R(const T&)>::type>(std::move(fun));
    }

    Observable<T> take(size_t index)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorTake<T>>(index)));
    }

    template<typename R>
    Observable<R> map(Function1UniquePtr<R, T>&& fun)
    {
        return lift(std::unique_ptr<Operator<T, R>>(make_unique<OperatorMap<T, R>>(std::move(fun))));
    }


    template<typename R>
    Observable<typename std::result_of<R(const T&)>::type> map(R&& fun)
    {
        return map<typename std::result_of<R(const T&)>::type>(std::move(fun));
    }


    template<typename K, typename V>
    Observable<std::map<K,V>> toMap(Function1UniquePtr<K, T>&& keySelector,
                                    Function1UniquePtr<V, T>&& valueSelector)
    {
        return lift(std::unique_ptr<Operator<T,
                    std::map<K,V>>>(make_unique<OperatorToMap<T, K, V>>(std::move(keySelector),
                    std::move(valueSelector))));
    }

    template<typename Ks, typename Vs>
    Observable<std::map<typename std::result_of<Ks(const T&)>::type,
    typename std::result_of<Vs(const T&)>::type>> toMap(Ks&& keySelector, Vs&& valueSelector)
    {
        return toMap<typename std::result_of<Ks(const T&)>::type, typename std::result_of<Vs(const T&)>::type>
                                                           (std::move(keySelector), std::move(valueSelector));
    }

    template<typename K>
    Observable<std::map<K,T>> toMap(Function1UniquePtr<K, T>&& keySelector)
    {
         return lift(std::unique_ptr<Operator<T,
                     std::map<K,T>>>(make_unique<OperatorToMap<T, K, T>>(std::move(keySelector),
                                     Function1UniquePtr<T,T>([](const T& t){return t;}))));
    }

    template<typename Ks>
    Observable<std::map<typename std::result_of<Ks(const T&)>::type, T>> toMap(Ks&& keySelector)
    {
         return toMap<typename std::result_of<Ks(const T&)>::type>(std::move(keySelector));
    }

    template<typename K, typename V>
    Observable<std::map<K,V>> toMap(Function1UniquePtr<K, T>&& keySelector,
                                    Function1UniquePtr<V, T>&& valueSelector,
                                    Function2UniquePtr<V, V, V>&& valuePrevSelector)
    {
        return lift(std::unique_ptr<Operator<T,
                    std::map<K,V>>>(make_unique<OperatorToMap<T, K, V>>(std::move(keySelector),
                    std::move(valueSelector), std::move(valuePrevSelector))));
    }

    template<typename Ks, typename Vs, typename Vps>
    Observable<std::map<typename std::result_of<Ks(const T&)>::type,
    typename std::result_of<Vs(const T&)>::type>> toMap(Ks&& keySelector, Vs&& valueSelector, Vps&& vpSelector)
    {
        return toMap<typename std::result_of<Ks(const T&)>::type, typename std::result_of<Vs(const T&)>::type>
                                               (std::move(keySelector), std::move(valueSelector), std::move(vpSelector));
    }

    Observable<bool> all(Predicat<T>&& pred)
    {
        return lift(std::unique_ptr<Operator<T, bool>>(make_unique<OperatorAll<T>>(std::move(pred))));
    }

    Observable<bool> exist(Predicat<T>&& pred)
    {
        return lift(std::unique_ptr<Operator<T, bool>>(make_unique<OperatorExist<T>>(std::move(pred))));
    }

    Observable<T> scan(Function2UniquePtr<T,T,T>&& accumulator)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorScan<T, T>>(std::move(accumulator))));
    }

    Observable<T> scan(Function2UniquePtr<T,T,T>&& accumulator, Function0UniquePtr<T>&& seed)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorScan<T, T>>(std::move(accumulator),
                                                                           std::move(seed))));
    }

    Observable<T> last()
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorLast<T>>()));
    }

    Observable<T> reduce(Function2UniquePtr<T,T,T>&& accumulator)
    {
        return scan(std::move(accumulator)).last();
    }

    template<typename R>
    Observable<R> concatMap(Function1UniquePtr<Observable<R>,T>&& mapper);

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
        worker->schedule(std::unique_ptr<Action0>(make_unique<ThreadAction>(source, subscriber)));;
    }
private:
    OnSubscribePtrType source;
    Scheduler::SchedulerRefType scheduler;
    Scheduler::WorkerRefType worker;
};

template<typename T>
Observable<T> Observable<T>::subscribeOn(Scheduler::SchedulerRefType&& scheduler)
{
    return create(std::shared_ptr<Observable<T>::OnSubscribe>(
                      std::make_shared<OperatorSubscribeOn<T>>(this->onSubscribe, std::move(scheduler))));
}

template<typename T, typename R>
class OnSubscribeConcatMap : public OnSubscribeBase<T>
{
public:
    using OnSubscribePtrType = std::shared_ptr<OnSubscribeBase<T>>;
    using MapperType = std::unique_ptr<Function1<Observable<R>,T>>;   //TODO replace with shared ptr
    using ThisChildSubscriberType = typename CompositeSubscriber<T,R>::ChildSubscriberType;

    struct ConcatMapSubscriber : public CompositeSubscriber<T,R>
    {
        ConcatMapSubscriber(ThisChildSubscriberType child, MapperType mapper) :
            CompositeSubscriber<T,R>(child), mapper(std::move(mapper))
        {}

        void onNext(const T& t) override
        {
            auto obs = (*mapper)(t);
            obs.subscribe(this->child);
        }

//        void onError(std::exception_ptr ex) override
//        {
//        }

//        void onComplete() override
//        {
//        }

        MapperType mapper;
    };


    OnSubscribeConcatMap(OnSubscribePtrType source, MapperType mapper) : source(source)
      ,mapper(std::move(mapper))
    {
    }

    void operator()(const SubscriberPtrType<R>& s) override
    {
        std::shared_ptr<ConcatMapSubscriber> parent = std::make_shared<ConcatMapSubscriber>(s, std::move(mapper));
        (*source)(parent);
    }

private:
    OnSubscribePtrType source;
    MapperType mapper;
};

template<typename T>
template<typename R>
Observable<R> Observable<T>::concatMap(Function1UniquePtr<Observable<R>,T>&& mapper)
{
    return create(std::make_shared<OnSubscribeConcatMap<T,R>>(this->onSubscribe, std::move(mapper)));
}


#endif // OBSERVABLE_H
