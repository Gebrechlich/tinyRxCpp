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
#include "operators/OperatorObserveOn.hpp"
#include "operators/OperatorToMap.hpp"
#include "operators/OperatorDoOnEach.hpp"
#include "operators/LiftOnSubscribe.hpp"
#include "operators/OperatorSubscribeOn.hpp"
#include "operators/DeferOnSubscribe.hpp"
#include "operators/RangeOnSubscribe.hpp"
#include "operators/RepeatOnSubscribe.hpp"
#include "operators/OnSubscribeConcatMap.hpp"
#include "operators/OnSubscribePeriodically.hpp"
#include "SchedulersFactory.hpp"
#include "utils/Util.hpp"
#include <memory>
#include <initializer_list>
#include <array>

template<typename T>
class Observable
{
public:
    using ThisSubscriberType = Subscriber<T>;
    using ThisSubscriberPtrType = SubscriberPtrType<T>;
    using OnSubscribe = OnSubscribeBase<T>;
    using ThisOnSubscribePtrType = std::shared_ptr<OnSubscribe>;

    template<typename R>
    using OnSubscribePtrType = std::shared_ptr<OnSubscribeBase<R>>;

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

    template<typename L>
    static Observable<T> from(const L& list)
    {
        static_assert((std::is_array<L>::value ||
                       is_iterable<L>::value), "Array type is required.");
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

    static Observable<T> just(T value)
    {
        return Observable<T>::create([value](const Observable<T>::
                                     ThisSubscriberPtrType& subscriber)
        {
            subscriber->onNext(value);
            subscriber->onComplete();
        });
    }

    static Observable<T> just(const T& t1, const T& t2)
    {
        std::array<T,2> list = {t1, t2};
        return Observable<T>::from(list);
    }

    static Observable<T> just(const T& t1, const T& t2, const T& t3)
    {
        std::array<T,3> list = {t1, t2, t3};
        return Observable<T>::from(list);
    }

    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4)
    {
        std::array<T,4> list = {t1, t2, t3, t4};
        return Observable<T>::from(list);
    }

    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4, const T& t5)
    {
        std::array<T,5> list = {t1, t2, t3, t4, t5};
        return Observable<T>::from(list);
    }

    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4,
                              const T& t5, const T& t6)
    {
        std::array<T,6> list = {t1, t2, t3, t4, t5, t6};
        return Observable<T>::from(list);
    }

    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4,
                              const T& t5, const T& t6, const T& t7)
    {
        std::array<T,7> list = {t1, t2, t3, t4, t5, t6, t7};
        return Observable<T>::from(list);
    }

    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4,
                              const T& t5, const T& t6, const T& t7, const T& t8)
    {
        std::array<T,8> list = {t1, t2, t3, t4, t5, t6, t7, t8};
        return Observable<T>::from(list);
    }

    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4,
                              const T& t5, const T& t6, const T& t7, const T& t8,
                              const T& t9)
    {
        std::array<T,9> list = {t1, t2, t3, t4, t5, t6, t7, t8, t9};
        return Observable<T>::from(list);
    }

    static Observable<T> just(const T& t1, const T& t2, const T& t3, const T& t4,
                              const T& t5, const T& t6, const T& t7, const T& t8,
                              const T& t9, const T& t10)
    {
        std::array<T,10> list = {t1, t2, t3, t4, t5, t6, t7, t8, t9, t10};
        return Observable<T>::from(list);
    }

    static Observable<T> range(T start, T count)
    {
        static_assert(std::is_integral<T>::value, "Integral type is required.");
        return create(std::make_shared<RangeOnSubscribe<T>>(start, count));
    }

    template<typename Rep, typename Period>
    static Observable<T> interval(const std::chrono::duration<Rep, Period>&  delay ,
                                  const std::chrono::duration<Rep, Period>&  period)
    {
        return create(std::shared_ptr<Observable<T>::OnSubscribe>(
                           std::make_shared<OnSubscribePeriodically<T, Rep, Period>>(
                           SchedulersFactory::instance().newThread(), delay, period)));
    }

    template<typename ObservableFactory>
    static Observable<T> defer(ObservableFactory&& observableFactory)
    {
        return create(std::shared_ptr<Observable<T>::OnSubscribe>(
                           std::make_shared<DeferOnSubscribe<T, ObservableFactory>>(
                           std::forward<ObservableFactory>(observableFactory))));
    }

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

    Observable<T> subscribeOn(Scheduler::SchedulerRefType&& scheduler)
    {
        return create(std::shared_ptr<Observable<T>::OnSubscribe>(
                          std::make_shared<OperatorSubscribeOn<T>>(this->onSubscribe, std::move(scheduler))));
    }

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
        typedef decltype(observableTypeTraits(mapper(T()))) R;
        return create<R>(std::make_shared<OnSubscribeConcatMap<T, R ,Mapper>>(this->onSubscribe, std::forward<Mapper>(mapper)));
    }

    static Observable<T> concat(Observable<Observable<T>>& observable)
    {
        return observable.concatMap([](const Observable<T>& o){
            return o;
        });
    }

    static Observable<T> concat(const Observable<T>& o1, const Observable<T>& o2)
    {
        auto o = std::move(Observable<Observable<T>>::just(o1, o2));
        return concat(o);
    }

    static Observable<T> concat(const Observable<T>& o1, const Observable<T>& o2, const Observable<T>& o3)
    {
         auto o = std::move(Observable<Observable<T>>::just(o1, o2, o3));
         return concat(o);
    }

    static Observable<T> concat(const Observable<T>& o1, const Observable<T>& o2, const Observable<T>& o3, const Observable<T>& o4)
    {
         auto o = std::move(Observable<Observable<T>>::just(o1, o2, o3, o4));
         return concat(o);
    }

    static Observable<T> concat(const Observable<T>& o1, const Observable<T>& o2, const Observable<T>& o3, const Observable<T>& o4,
                                const Observable<T>& o5)
    {
         auto o = std::move(Observable<Observable<T>>::just(o1, o2, o3, o4, o5));
         return concat(o);
    }

    static Observable<T> concat(const Observable<T>& o1, const Observable<T>& o2, const Observable<T>& o3, const Observable<T>& o4,
                                const Observable<T>& o5, const Observable<T>& o6)
    {
         auto o = std::move(Observable<Observable<T>>::just(o1, o2, o3, o4, o5, o6));
         return concat(o);
    }

    static Observable<T> concat(const Observable<T>& o1, const Observable<T>& o2, const Observable<T>& o3, const Observable<T>& o4,
                                const Observable<T>& o5, const Observable<T>& o6, const Observable<T>& o7)
    {
         auto o = std::move(Observable<Observable<T>>::just(o1, o2, o3, o4, o5, o6, o7));
         return concat(o);
    }

    Observable<T> repeat(size_t count = 0)
    {
        return create<T>(std::make_shared<RepeatOnSubscribe<T>>(this->onSubscribe, count));
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

#endif // OBSERVABLE_H
