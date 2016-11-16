#ifndef OPERATORDISTINCT_H
#define OPERATORDISTINCT_H

#include "Operator.hpp"
#include <type_traits>
#include <utility>
#include <unordered_set>
#include <mutex>

template<typename T>
T asIs(const T& t)
{
    return t;
}

template<typename T, typename KeyGen>
class OperatorDistinct : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType   = typename CompositeSubscriber<T,T>::ChildSubscriberType;
    using KeyGenType           = typename std::decay<KeyGen>::type;
    using KeyType              = typename std::result_of<KeyGen(const T&)>::type;

    struct DistinctSubscriber : public CompositeSubscriber<T,T>
    {
        DistinctSubscriber(ThisSubscriberType p, KeyGenType&& kG) :
            CompositeSubscriber<T,T>(p), keyGenerator(std::move(kG))
        {
        }

        void onNext(const T& t) override
        {
            KeyType k = keyGenerator(t);
            std::lock_guard<std::mutex> ul(mut);
            if(values.insert(k).second)
            {
                this->child->onNext(t);
            }
        }

        std::unordered_set<KeyType> values;
        KeyGenType keyGenerator;
        std::mutex mut;
    };

public:

    OperatorDistinct(const KeyGenType& kG) : Operator<T,T>(), keyGenerator(kG)
    {}

    OperatorDistinct(KeyGenType&& kG) : Operator<T,T>(), keyGenerator(std::move(kG))
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
