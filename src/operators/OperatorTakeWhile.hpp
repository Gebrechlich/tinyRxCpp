#ifndef OPERATORTAKEWHILE_HPP
#define OPERATORTAKEWHILE_HPP

#include "Operator.hpp"

template<typename T, typename Predicate>
class OperatorTakeWhile : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType   = typename CompositeSubscriber<T,T>::ChildSubscriberType;
    using PredicateType        = typename std::decay<Predicate>::type;

    struct TakeWhileSubscriber : public CompositeSubscriber<T,T>
    {
        TakeWhileSubscriber(ThisSubscriberType p, PredicateType&& pred) :
            CompositeSubscriber<T,T>(p), predicate(std::move(pred))
        {}

        void onNext(const T& t) override
        {
            if(predicate(t))
            {
               this->child->onNext(t);
            }
            else
            {
                done = true;
                this->child->onComplete();
                this->unsubscribe();
            }
        }

        void onError(std::exception_ptr ex) override
        {
            if(!done)
            {
                this->child->onError(ex);
            }
        }

        void onComplete() override
        {
            if(!done)
            {
                this->child->onComplete();
            }
        }

        PredicateType predicate;
        volatile bool done = false;
    };

public:
    OperatorTakeWhile(const PredicateType& pred) : Operator<T,T>(),
        predicate(pred)
    {}

    OperatorTakeWhile(PredicateType&& pred) : Operator<T,T>(),
        predicate(std::move(pred))
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<TakeWhileSubscriber>(t, std::move(predicate));
        subs->addChildSubscriptionFromThis();
        return subs;
    }
private:
    PredicateType predicate;
};


#endif // OPERATORTAKEWHILE_HPP
