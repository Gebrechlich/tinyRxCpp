#ifndef OPERATORDOONEACH_HPP
#define OPERATORDOONEACH_HPP
#include "Operator.hpp"

template<typename T>
class OperatorDoOnEach : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    struct DoOnEachSubscriber : public CompositeSubscriber<T,T>
    {
        DoOnEachSubscriber(ThisSubscriberType p, Action1RefType<T> onNext, Action1RefType<std::exception_ptr> onError,
                           ActionRefType onComplete) : CompositeSubscriber<T,T>(p),
            onNextAct(onNext), onErrorAct(onError), onCompleteAct(onComplete)
        {}

        void onNext(const T& t) override
        {
            (*onNextAct)(t);
            this->child->onNext(t);
        }

        void onError(std::exception_ptr ex) override
        {
            (*onErrorAct)(ex);
            this->child->onError(ex);
        }

        void onComplete() override
        {
            (*onCompleteAct)();
            this->child->onComplete();
        }

        Action1RefType<T> onNextAct;
        Action1RefType<std::exception_ptr> onErrorAct;
        ActionRefType onCompleteAct;
    };

public:
    OperatorDoOnEach(Action1RefType<T> onNext, Action1RefType<std::exception_ptr> onError,
                      ActionRefType onComplete) : Operator<T,T>(),
        onNext(onNext), onError(onError), onComplete(onComplete)

    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<DoOnEachSubscriber>(t, onNext, onError, onComplete);
        return subs;
    }
private:
    Action1RefType<T> onNext;
    Action1RefType<std::exception_ptr> onError;
    ActionRefType onComplete;
};

#endif // OPERATORDOONEACH_HPP
