#ifndef OPERATORALL_HPP
#define OPERATORALL_HPP
#include "Operator.hpp"
#include <type_traits>

template<typename T, typename Predicate>
class OperatorAll : public Operator<T,bool>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType   = typename CompositeSubscriber<T,bool>::ChildSubscriberType;
    using PredicateType        = typename std::decay<Predicate>::type;

    struct AllSubscriber : public CompositeSubscriber<T,bool>
    {
        AllSubscriber(ThisSubscriberType p, PredicateType&& pred) :
            CompositeSubscriber<T,bool>(p), predicate(std::move(pred))
        {
        }

        void onNext(const T& t) override
        {
            if(!predicate(t) && !done)
            {
                done = true;
                this->child->onNext(false);
            }
        }

        void onComplete() override
        {
            this->child->onComplete();
            if(!done)
            {
                this->child->onNext(true);
                done = true;
            }
        }

        PredicateType predicate;
        bool done = false;
    };

public:
    OperatorAll(const PredicateType& pred) : Operator<T,bool>(),
        predicate(pred)
    {}

    OperatorAll(PredicateType&& pred) : Operator<T,bool>(),
        predicate(std::move(pred))
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<AllSubscriber>(t, std::move(predicate));
        subs->addChildSubscriptionFromThis();
        return subs;
    }
private:
    PredicateType predicate;
};

#endif // OPERATORALL_HPP

