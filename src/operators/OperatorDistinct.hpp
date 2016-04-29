#ifndef OPERATORDISTINCT_H
#define OPERATORDISTINCT_H

#include "Operator.hpp"
#include <utility>
#include <set>

template<typename T>
T asIs(const T& t)
{
    return t;
}

template<typename T, typename R>
class OperatorDistinct : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;
    using KeyGenType = Function1_t<R ,T>;

    struct DistinctSubscriber : public CompositeSubscriber<T,T>
    {
        DistinctSubscriber(ThisSubscriberType p, KeyGenType kG) :
            CompositeSubscriber<T,T>(p), keyGenerator(std::move(kG))
        {
        }

        void onNext(const T& t) override
        {
            R k = keyGenerator(t);
            if(values.insert(k).second)
            {
                this->child->onNext(t);
            }
        }

        std::set<R> values;
        KeyGenType keyGenerator;
    };

public:
    OperatorDistinct(KeyGenType kG) : Operator<T,T>(), keyGenerator(std::move(kG))
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<DistinctSubscriber>(t, std::move(keyGenerator));
        subs->addChildSubscriptionFromThis();
        return subs;
    }

    KeyGenType keyGenerator;
};

#endif // OPERATORDISTINCT_H
