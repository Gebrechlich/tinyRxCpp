#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include "Functions.hpp"
#include "Subscriber.hpp"
#include "operators/OperatorFilter.hpp"
#include "operators/OperatorMap.hpp"
#include "operators/OperatorDistinct.hpp"
#include "operators/OperatorAll.hpp"
#include "operators/OperatorExist.hpp"
#include "operators/OperatorScan.hpp"
#include "operators/OperatorLast.hpp"
#include "operators/OperatorTake.hpp"
#include "operators/OperatorTakeWhile.hpp"
#include "operators/OperatorObserveOn.hpp"
#include "operators/OperatorToMap.hpp"
#include "operators/OperatorDoOnEach.hpp"
#include "operators/LiftOnSubscribe.hpp"
#include "operators/OperatorSubscribeOn.hpp"
#include "operators/DeferOnSubscribe.hpp"
#include "operators/RangeOnSubscribe.hpp"
#include "operators/RepeatOnSubscribe.hpp"
#include "operators/OnSubscribeConcatMap.hpp"
#include "operators/OnSubscribeFlatMap.hpp"
#include "operators/OnSubscribePeriodically.hpp"
#include "SchedulersFactory.hpp"
#include "utils/Util.hpp"
#include <memory>
#include <initializer_list>
#include <array>


template<typename T>
using OnSubscribePtrType = std::shared_ptr<OnSubscribeBase<T>>;

template<typename T = void>
class Observable
{
public:
    using ThisSubscriberType = Subscriber<T>;
    using ThisSubscriberPtrType = SubscriberPtrType<T>;
    using OnSubscribe = OnSubscribeBase<T>;
    using ThisOnSubscribePtrType = std::shared_ptr<OnSubscribe>;
    using ValueType = T;

    Observable() = default;
    ~Observable() = default;

    Observable(ThisOnSubscribePtrType f) : onSubscribe(std::move(f))
    {
    }

    template<typename R>
    static Observable<R> create(OnSubscribePtrType<R> onSub)
    {
        return Observable<R>(std::move(onSub));
    }

    template<typename R>
    static Observable<R> create(typename OnSubscribeBase<R>::ActionFp action)
    {
        return Observable<R>(OnSubscribePtrType<R>(std::make_shared<OnSubscribeBase<R>>(action)));
    }

    static Observable<T> create(ThisOnSubscribePtrType onSub)
    {
        return create<T>(std::move(onSub));
    }

    static Observable<T> create(typename OnSubscribe::ActionFp action)
    {
        return create<T>(ThisOnSubscribePtrType(std::make_shared<OnSubscribe>(action)));
    }

    SubscriptionPtrType subscribe(typename ThisSubscriberType::ThisOnNextFP next)
    {
        return Observable<T>::subscribe(std::move(createSubscriber(next,
                       typename ThisSubscriberType::ThisOnErrorFP(),
                       typename ThisSubscriberType::ThisOnCompleteFP())), this);
    }

    SubscriptionPtrType subscribe(typename ThisSubscriberType::ThisOnNextFP next,
                   typename ThisSubscriberType::ThisOnCompleteFP complete)
    {
        return Observable<T>::subscribe(std::move(createSubscriber(next,
                   typename ThisSubscriberType::ThisOnErrorFP(), complete)), this);
    }

    SubscriptionPtrType subscribe(typename ThisSubscriberType::ThisOnNextFP next,
                   typename ThisSubscriberType::ThisOnErrorFP error,
                   typename ThisSubscriberType::ThisOnCompleteFP complete)
    {
        return Observable<T>::subscribe(std::move(createSubscriber(next, error, complete)), this);
    }

    SubscriptionPtrType subscribe(typename ThisSubscriberType::ThisOnNextFP next,
                   typename ThisSubscriberType::ThisOnErrorFP error)
    {
        return Observable<T>::subscribe(std::move(createSubscriber(next, error,
                   typename ThisSubscriberType::ThisOnCompleteFP())), this);
    }

    SubscriptionPtrType subscribe(ThisSubscriberPtrType subscriber)
    {
        return Observable<T>::subscribe(subscriber, this);
    }

    Observable<T> subscribeOn(const Scheduler::SchedulerRefType& scheduler)
    {
        return create(std::shared_ptr<Observable<T>::OnSubscribe>(
                          std::make_shared<OperatorSubscribeOn<T>>(this->onSubscribe, scheduler)));
    }

    Observable<T> observeOn(const Scheduler::SchedulerRefType& scheduler)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorObserveOn<T>>(scheduler,
                                                                                      std::numeric_limits<int>::max())),
                    this->onSubscribe);
    }

    Observable<T> observeOn(const Scheduler::SchedulerRefType& scheduler, size_t bufferSize)
    {
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorObserveOn<T>>(scheduler, bufferSize)),
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
        static_assert(std::is_same<typename std::result_of<Predicate(const T&)>::type, bool>::value,
                      "Predicate(T&) must return a bool value");
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

    template<typename Predicate>
    Observable<T> takeWhile(Predicate&& pred)
    {
        static_assert(std::is_same<typename std::result_of<Predicate(const T&)>::type, bool>::value,
                      "Predicate(T&) must return a bool value");
        return lift(std::unique_ptr<Operator<T, T>>(make_unique<OperatorTakeWhile<T, Predicate>>
                                                   (std::forward<Predicate>(pred))));
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
        static_assert(std::is_same<typename std::result_of<Predicate(const T&)>::type, bool>::value,
                      "Predicate(T&) must return a bool value");
        return lift(std::unique_ptr<Operator<T, bool>>(make_unique<OperatorAll<T, Predicate>>(std::forward<Predicate>(pred))));
    }

    template<typename Predicate>
    Observable<bool> exist(Predicate&& pred)
    {
        static_assert(std::is_same<typename std::result_of<Predicate(const T&)>::type, bool>::value,
                      "Predicate(T&) must return a bool value");
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
    typename std::result_of<Mapper(const T&)>::type concatMap(Mapper&& mapper)
    {
        typedef decltype(mapper(T())) ObservableType;
        typedef typename ObservableType::ValueType Type;
        return create<Type>(std::make_shared<OnSubscribeConcatMap<T, Type, Mapper>>(this->onSubscribe, std::forward<Mapper>(mapper)));
    }

    template<typename Mapper>
    typename std::result_of<Mapper(const T&)>::type flatMap(Mapper&& mapper)
    {
         typedef decltype(mapper(T())) ObservableType;
         typedef typename ObservableType::ValueType Type;
         return create<Type>(std::make_shared<OnSubscribeFlatMap<T, Type, Mapper>>(this->onSubscribe, std::forward<Mapper>(mapper)));
    }

    Observable<T> repeat(size_t count = 0)
    {
        return create<T>(std::make_shared<RepeatOnSubscribe<T>>(this->onSubscribe, count));
    }

protected:
    template<typename B>
    static SubscriptionPtrType subscribe(SubscriberPtrType<B> subscriber,
                          Observable<B>* observable)
    {
        std::weak_ptr<SubscriptionBase> ptr = subscriber;
        SubscriptionPtrType subs = std::make_shared<WeekSubscription>(std::move(ptr));
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

template<>
class Observable<void>
{
public:
    ~Observable() = delete;

    template<typename T>
    using ThisOnSubscribePtrType = std::shared_ptr<OnSubscribeBase<T>>;

    template<typename T>
    static Observable<T> create(OnSubscribePtrType<T> onSub)
    {
        return Observable<T>(std::move(onSub));
    }

    template<typename T>
    static Observable<T> create(typename OnSubscribeBase<T>::ActionFp action)
    {
        return Observable<T>(OnSubscribePtrType<T>(std::make_shared<OnSubscribeBase<T>>(action)));
    }

    template<typename L>
    static auto from(const L& list) ->
    Observable<typename std::remove_cv<typename std::remove_reference<decltype(resolveConteinerValueType(list))>::type>::type>

    {
        typedef typename std::remove_cv<typename std::remove_reference<decltype(resolveConteinerValueType(list))>::type>::type type;
        return fromInner<type>(list);
    }

    template<typename T>
    static Observable<T> just(T value)
    {
        return Observable<T>::create([value](const typename Observable<T>::
                                     ThisSubscriberPtrType& subscriber)
        {
            subscriber->onNext(value);
            subscriber->onComplete();
        });
    }

    template<typename T>
    static Observable<T> just(const T& t1, const T& t2)
    {
        std::array<T,2> list = {t1, t2};
        return from(list);
    }

    template<typename T>
    static Observable<T> just(const T& t1, const T& t2, const T& t3)
    {
        std::array<T,3> list = {t1, t2, t3};
        return from(list);
    }

    template<typename T>
    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4)
    {
        std::array<T,4> list = {t1, t2, t3, t4};
        return from(list);
    }

    template<typename T>
    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4, const T& t5)
    {
        std::array<T,5> list = {t1, t2, t3, t4, t5};
        return from(list);
    }

    template<typename T>
    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4,
                              const T& t5, const T& t6)
    {
        std::array<T,6> list = {t1, t2, t3, t4, t5, t6};
        return from(list);
    }

    template<typename T>
    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4,
                              const T& t5, const T& t6, const T& t7)
    {
        std::array<T,7> list = {t1, t2, t3, t4, t5, t6, t7};
        return from(list);
    }

    template<typename T>
    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4,
                              const T& t5, const T& t6, const T& t7, const T& t8)
    {
        std::array<T,8> list = {t1, t2, t3, t4, t5, t6, t7, t8};
        return from(list);
    }

    template<typename T>
    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4,
                              const T& t5, const T& t6, const T& t7, const T& t8,
                              const T& t9)
    {
        std::array<T,9> list = {t1, t2, t3, t4, t5, t6, t7, t8, t9};
        return from(list);
    }

    template<typename T>
    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4,
                              const T& t5, const T& t6, const T& t7, const T& t8,
                              const T& t9, const T& t10)
    {
        std::array<T,10> list = {t1, t2, t3, t4, t5, t6, t7, t8, t9, t10};
        return from(list);
    }

    template<typename T>
    static Observable<T> range(T start, T count)
    {
        static_assert(std::is_integral<T>::value, "Integral type is required.");
        return create<T>(std::make_shared<RangeOnSubscribe<T>>(start, count));
    }

    template<typename ObservableFactory>
    static auto defer(ObservableFactory&& observableFactory) ->
    typename std::result_of<ObservableFactory()>::type
    {
        typedef typename std::result_of<ObservableFactory()>::type ObservableType;
        typedef typename ObservableType::ValueType Type;
        return create<Type>(std::make_shared<DeferOnSubscribe<Type,
                            ObservableFactory>>(std::forward<ObservableFactory>(observableFactory)));
    }

    template<typename Rep, typename Period>
    static Observable<size_t> interval(const std::chrono::duration<Rep, Period>&  delay ,
                                       const std::chrono::duration<Rep, Period>&  period,
                                       size_t count = std::numeric_limits<size_t>::max())
    {
        return create(std::shared_ptr<Observable<size_t>::OnSubscribe>(
                           std::make_shared<OnSubscribePeriodically<size_t, Rep, Period>>(
                           SchedulersFactory::instance().newThread(), delay, period, count)));
    }

    template<typename Rep, typename Period>
    static Observable<size_t> interval(const std::chrono::duration<Rep, Period>&  period)
    {
        return interval(std::chrono::duration<Rep, Period>(0), period);
    }

    template<typename Rep, typename Period>
    static Observable<size_t> time(const std::chrono::duration<Rep, Period>&  delay)
    {
        return interval(std::chrono::duration<Rep, Period>(delay), std::chrono::duration<Rep, Period>(0), 1);
    }

    template<typename T>
    static Observable<T> concat(Observable<Observable<T>>& observable)
    {
        return observable.concatMap([](const Observable<T>& o){
            return o;
        });
    }

    template<typename T, typename ...R>
    static Observable<T> concat(const Observable<T>& a, const Observable<R>& ...args)
    {
        auto o = std::move(just(a, args...));
        return concat(o);
    }

    template<typename T>
    static Observable<T> merge(Observable<Observable<T>>& observable)
    {
        return observable.flatMap([](const Observable<T>& o){
            return o;
        });
    }

    template<typename T, typename ...R>
    static Observable<T> merge(const Observable<T>& a, const Observable<R>& ...args)
    {
        auto o = std::move(just(a, args...));
        return merge(o);
    }

private:
    template<typename T, typename L>
    static Observable<T> fromInner(const L& list)
    {
        static_assert((std::is_array<L>::value ||
                       is_iterable<L>::value), "Array type is required.");

        return Observable<T>::create([list](const typename Observable<T>::
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
};

#endif // OBSERVABLE_H
