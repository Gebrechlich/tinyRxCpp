#ifndef OPERATORFILTER_H
#define OPERATORFILTER_H
#include "Operator.hpp"

template<typename T, typename Predicate>
class OperatorFilter : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType   = typename CompositeSubscriber<T,T>::ChildSubscriberType;
    using PredicateType        = typename std::decay<Predicate>::type;

    struct FilterSubscriber : public CompositeSubscriber<T,T>
    {
        FilterSubscriber(ThisSubscriberType p, PredicateType&& pred) :
            CompositeSubscriber<T,T>(p), predicate(std::move(pred))
        {}

        void onNext(const T& t) override
        {
            if(predicate(t))
            {
               this->child->onNext(t);
            }
        }

        PredicateType predicate;
    };

public:
    OperatorFilter(const PredicateType& pred) : Operator<T,T>(),
        predicate(pred)
    {}

    OperatorFilter(PredicateType&& pred) : Operator<T,T>(),
        predicate(std::move(pred))
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<FilterSubscriber>(t, std::move(predicate));
        subs->addChildSubscriptionFromThis();
        return subs;
    }
private:
    PredicateType predicate;
};


#endif // OPERATORFILTER_H
