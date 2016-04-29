#ifndef OPERATORFILTER_H
#define OPERATORFILTER_H
#include "Operator.hpp"

template<typename T>
class OperatorFilter : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    struct FilterSubscriber : public CompositeSubscriber<T,T>
    {
        FilterSubscriber(ThisSubscriberType p, Predicat<T> pred) :
            CompositeSubscriber<T,T>(p), predicate(std::move(pred))
        {}

        void onNext(const T& t) override
        {
            if(predicate(t))
            {
               this->child->onNext(t);
            }
        }

        Predicat<T> predicate;
    };

public:
    OperatorFilter(Predicat<T> pred) : Operator<T,T>(),
        predicate(std::move(pred))
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<FilterSubscriber>(t, std::move(predicate));
        subs->addChildSubscriptionFromThis();
        return subs;
    }
private:
    Predicat<T> predicate;
};


#endif // OPERATORFILTER_H
