#ifndef OPERATORDOONEACH_HPP
#define OPERATORDOONEACH_HPP
#include "Operator.hpp"
#include <type_traits>

template<typename T, typename OnNext, typename OnError, typename OnComplete>
class OperatorDoOnEach : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType   = typename CompositeSubscriber<T,T>::ChildSubscriberType;
    using OnNextType           = typename std::decay<OnNext>::type;
    using OnErrorType          = typename std::decay<OnError>::type;
    using OnCopmleteType       = typename std::decay<OnComplete>::type;

    struct DoOnEachSubscriber : public CompositeSubscriber<T,T>
    {
        DoOnEachSubscriber(ThisSubscriberType p, OnNextType&& onNext, OnErrorType&& onError,
                           OnCopmleteType&& onComplete) : CompositeSubscriber<T,T>(p),
            onNextAct(std::move(onNext)), onErrorAct(std::move(onError)), onCompleteAct(std::move(onComplete))
        {}

        void onNext(const T& t) override
        {
            onNextAct(t);
            this->child->onNext(t);
        }

        void onError(std::exception_ptr ex) override
        {
            onErrorAct(ex);
            this->child->onError(ex);
        }

        void onComplete() override
        {
            onCompleteAct();
            this->child->onComplete();
        }

        OnNextType onNextAct;
        OnErrorType onErrorAct;
        OnCopmleteType onCompleteAct;
    };

public:
    OperatorDoOnEach(OnNextType onNext, OnErrorType onError,
                       OnCopmleteType onComplete) : Operator<T,T>(),
        onNext(std::move(onNext)), onError(std::move(onError)), onComplete(std::move(onComplete))

    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<DoOnEachSubscriber>(t, std::move(onNext), std::move(onError), std::move(onComplete));
        subs->addChildSubscriptionFromThis();
        return subs;
    }
private:
    OnNextType onNext;
    OnErrorType onError;
    OnCopmleteType onComplete;
};

#endif // OPERATORDOONEACH_HPP
