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
#include "OperatorDoOnEach.hpp"
#include "Functions.hpp"
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

template<typename T, typename ObservableFactory>
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

    template<typename U, typename R, typename Mapper>
    friend class OnSubscribeConcatMap;

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

    template<typename ObservableFactory>
    static Observable<T> defer(ObservableFactory&& observableFactory);

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

    template<typename Predicate>
    Observable<T> filter(Predicate&& pred)
    {
        static_assert(std::is_same<typename std::result_of<Predicate(const T&)>::type, bool>::value, "Predicate(T&) must return a bool value");
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorFilter<T, Predicate>>
                                                   (std::forward<Predicate>(pred))));
    }

    template<typename KeyGen>
    Observable<T> distinct(KeyGen&& keyGenerator)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorDistinct<T,KeyGen>>(std::forward<KeyGen>(keyGenerator))));
    }

    Observable<T> distinct()
    {
        return distinct(asIs<T>);
    }

    Observable<T> take(size_t index)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorTake<T>>(index)));
    }

    template<typename OnNext, typename OnError, typename OnComplete>
    Observable<T> doOnEach(OnNext&& onNext, OnError&& onError, OnComplete&& onComplete)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorDoOnEach<T, OnNext, OnError, OnComplete>>
                                                   (std::forward<OnNext>(onNext),
                                                    std::forward<OnError>(onError),
                                                    std::forward<OnComplete>(onComplete))));
    }

    template<typename OnNext>
    Observable<T> doOnNext(OnNext&& onNext)
    {
        return doOnEach(std::forward<OnNext>(onNext), [](std::exception_ptr){}, [](){});
    }

    template<typename OnError>
    Observable<T> doOnError(OnError&& onError)
    {
        return doOnEach([](const T&){}, std::forward<OnError>(onError), [](){});
    }

    template<typename OnComplete>
    Observable<T> doOnCompleted(OnComplete&& onComplete)
    {
        return doOnEach([](const T&){}, [](std::exception_ptr){}, std::forward<OnComplete>(onComplete));
    }

    template<typename Mapper>
    Observable<typename std::result_of<Mapper(const T&)>::type> map(Mapper&& fun)
    {
        return lift(std::unique_ptr<Operator<T, typename std::result_of<Mapper(const T&)>::type>>
                    (make_unique<OperatorMap<T, Mapper>>(std::forward<Mapper>(fun))));
    }

    template<typename KeySelector, typename ValueSelector, typename ValuePrevSelector>
    Observable<MapT<T,KeySelector,ValueSelector>>
    toMap(KeySelector&& keySelector, ValueSelector&& valueSelector, ValuePrevSelector&& vpSelector)
    {
        return lift(std::unique_ptr<Operator<T,MapT<T,KeySelector,ValueSelector>>>(
                   make_unique<OperatorToMap<T, KeySelector, ValueSelector, ValuePrevSelector>>(
                                                          std::forward<KeySelector>(keySelector),
                                                          std::forward<ValueSelector>(valueSelector),
                                                          std::forward<ValuePrevSelector>(vpSelector))));
    }

    template<typename KeySelector, typename ValueSelector>
    Observable<MapT<T,KeySelector,ValueSelector>> toMap(KeySelector&& keySelector, ValueSelector&& valueSelector)
    {
        return toMap(std::forward<KeySelector>(keySelector), std::forward<ValueSelector>(valueSelector),
                                    [](const typename std::result_of<ValueSelector(const T&)>::type& t,
                                    typename std::result_of<ValueSelector(const T&)>::type){return t;});
    }

    template<typename KeySelector>
    Observable<std::map<typename std::result_of<KeySelector(const T&)>::type, T>> toMap(KeySelector&& keySelector)
    {
        return toMap(std::forward<KeySelector>(keySelector),
                     [](const T& t){return t;}, [](const T& t,const T&){return t;});
    }

    template<typename Predicate>
    Observable<bool> all(Predicate&& pred)
    {
        static_assert(std::is_same<typename std::result_of<Predicate(const T&)>::type, bool>::value, "Predicate(T&) must return a bool value");
        return lift(std::unique_ptr<Operator<T, bool>>(make_unique<OperatorAll<T, Predicate>>(std::forward<Predicate>(pred))));
    }

    template<typename Predicate>
    Observable<bool> exist(Predicate&& pred)
    {
        static_assert(std::is_same<typename std::result_of<Predicate(const T&)>::type, bool>::value, "Predicate(T&) must return a bool value");
        return lift(std::unique_ptr<Operator<T, bool>>(make_unique<OperatorExist<T, Predicate>>(std::forward<Predicate>(pred))));
    }

    template<typename Accumulator>
    Observable<typename std::result_of<Accumulator(const T&, const T&)>::type> scan(Accumulator&& accumulator)
    {
        return lift(std::unique_ptr<Operator<T,typename std::result_of<Accumulator(const T&, const T&)>::type>>(
                        make_unique<OperatorScan<T, Accumulator>>(std::forward<Accumulator>(accumulator))));
    }

    template<typename Accumulator>
    Observable<typename std::result_of<Accumulator(const T&, const T&)>::type> scan(Accumulator&& accumulator, const T& seed)
    {
        return lift(std::unique_ptr<Operator<T,typename std::result_of<Accumulator(const T&, const T&)>::type>>(
                        make_unique<OperatorScan<T, Accumulator>>(std::forward<Accumulator>(accumulator), seed, true)));
    }

    Observable<T> last()
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorLast<T>>()));
    }

    template<typename Accumulator>
    Observable<T> reduce(Accumulator&& accumulator)
    {
        return scan(std::forward<Accumulator>(accumulator)).last();
    }

    template<typename Mapper>
    typename std::result_of<Mapper(const T&)>::type concatMap(Mapper&& mapper);

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

template<typename T, typename ObservableFactory>
class DeferOnSubscribe : public OnSubscribeBase<T>
{
public:
    DeferOnSubscribe(const ObservableFactory& observFactory) :
        observableFactory(observFactory)
    {}

    DeferOnSubscribe(ObservableFactory&& observFactory) :
        observableFactory(std::move(observFactory))
    {}

    void operator()(const SubscriberPtrType<T>& t) override
    {
        auto o = observableFactory();
        o.subscribe(std::move(const_cast<SubscriberPtrType<T>&>(t)));
    }
private:
    ObservableFactory observableFactory;
};

template<typename T>
template<typename ObservableFactory>
Observable<T> Observable<T>::defer(ObservableFactory &&observableFactory)
{
    return create(std::shared_ptr<Observable<T>::OnSubscribe>(
                       std::make_shared<DeferOnSubscribe<T, ObservableFactory>>(
                       std::forward<ObservableFactory>(observableFactory))));
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

template<typename T, typename R, typename Mapper>
class OnSubscribeConcatMap : public OnSubscribeBase<T>
{
public:
    using OnSubscribePtrType      = std::shared_ptr<OnSubscribeBase<T>>;
    using MapperType              = typename std::decay<Mapper>::type;
    using ThisChildSubscriberType = typename CompositeSubscriber<T,R>::ChildSubscriberType;

    struct InnerConcatMapSubscriber;

    struct ConcatMapSubscriber : public CompositeSubscriber<T,R>
    {
        ConcatMapSubscriber(ThisChildSubscriberType child, MapperType&& mapper) :
            CompositeSubscriber<T,R>(child), mapper(std::move(mapper)), requested(0)
        {}

        void onNext(const T& t) override
        {
            ++requested;
            if(!this->isUnsubscribe())
            {
                auto obs = mapper(t);
                std::shared_ptr<Subscriber<T>> innerSubscriber = std::make_shared<InnerConcatMapSubscriber>
                        (std::dynamic_pointer_cast<ConcatMapSubscriber>(this->shared_from_this()));

                obs.subscribe(innerSubscriber);
                this->add(SharedSubscription(innerSubscriber));
            }
        }

        void onNextInner(const T& t)
        {
            this->child->onNext(t);
        }

        void onErrorInner(std::exception_ptr ex)
        {
            this->child->onError(ex);
        }

        void onCompleteInner()
        {
            --requested;
            if(parentComplete && !done && requested.load() == 0)
            {
                done = true;
                this->child->onComplete();
            }
        }

        void onComplete() override
        {
            parentComplete = true;
            if(requested.load() == 0 && !done)
            {
                done = true;
                this->child->onComplete();
            }
        }

        MapperType mapper;
        std::atomic_int requested;
        volatile bool parentComplete = false;
        volatile bool done = false;
    };

    struct InnerConcatMapSubscriber : public Subscriber<T>
    {
        InnerConcatMapSubscriber(std::shared_ptr<ConcatMapSubscriber> child) : child(child)
        {}

        void onNext(const T& t) override
        {
            child->onNextInner(t);
        }

        void onError(std::exception_ptr ex) override
        {
            child->onErrorInner(ex);
        }

        void onComplete() override
        {
            child->onCompleteInner();
        }

        std::shared_ptr<ConcatMapSubscriber> child;
    };

    OnSubscribeConcatMap(OnSubscribePtrType source, const MapperType& mapper) : source(source)
      ,mapper(mapper)
    {}

    OnSubscribeConcatMap(OnSubscribePtrType source, MapperType&& mapper) : source(source)
      ,mapper(std::move(mapper))
    {}

    void operator()(const SubscriberPtrType<R>& s) override
    {
        if(s == nullptr)
        {
            return;
        }

        std::shared_ptr<ConcatMapSubscriber> parent = std::make_shared<ConcatMapSubscriber>(s, std::move(mapper));
        s->add(SharedSubscription(parent));

        if(!s->isUnsubscribe())
        {
            (*source)(parent);
        }
    }

private:
    OnSubscribePtrType source;
    MapperType mapper;
};

template<typename T>
template<typename Mapper>
typename std::result_of<Mapper(const T&)>::type Observable<T>::concatMap(Mapper&& mapper)
{
    typedef decltype(observableTypeTraits(mapper(T()))) R;
    return create(std::make_shared<OnSubscribeConcatMap<T, R ,Mapper>>(
                      this->onSubscribe, std::forward<Mapper>(mapper)));
}

#endif // OBSERVABLE_H
