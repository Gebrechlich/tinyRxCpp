#ifndef OPERATORTOMAP_HPP
#define OPERATORTOMAP_HPP
#include "Operator.hpp"
#include <map>

template<typename T, typename K, typename V>
class OperatorToMap : public Operator<T, std::map<K, V>>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using MapType =  std::map<K, V>;
    using ThisSubscriberType = typename CompositeSubscriber<T, MapType>::ChildSubscriberType;
    using KeySelector = std::unique_ptr<Function1<K, T>>;
    using ValueSelector = std::unique_ptr<Function1<V, T>>;
    using ValuePrevSelector = std::unique_ptr<Function2<V, V, V>>;

    struct ToMapSubscriber : public CompositeSubscriber<T, MapType>
    {
        ToMapSubscriber(ThisSubscriberType p, KeySelector kSelector, ValueSelector vSelector
                        ,ValuePrevSelector vpSelector = nullptr) :
            CompositeSubscriber<T, MapType>(p), keySelector(std::move(kSelector)),
            valueSelector(std::move(vSelector)), valuePrevSelector(std::move(vpSelector))
        {
        }

        void onNext(const T& t) override
        {
            auto value = (*valueSelector)(t);
            auto key = (*keySelector)(t);
            if(valuePrevSelector)
            {
                auto prev = (map.find(key) == map.end() ? value : map[key]);
                map[key] = (*valuePrevSelector)(prev, value);
            }
            else
            {
                map[key] = value;
            }
        }

        void onComplete() override
        {
            this->child->onNext(map);
            this->child->onComplete();
        }

        KeySelector keySelector;
        ValueSelector valueSelector;
        ValuePrevSelector valuePrevSelector;
        MapType map;
        bool prevPas;
    };

public:
    OperatorToMap(){}
    OperatorToMap(KeySelector kSelector, ValueSelector vSelector, ValuePrevSelector vpSelector = nullptr) : Operator<T, MapType>(),
        keySelector(std::move(kSelector)), valueSelector(std::move(vSelector)), valuePrevSelector(std::move(vpSelector))
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<ToMapSubscriber>(t, std::move(keySelector),
                         std::move(valueSelector), std::move(valuePrevSelector));
        subs->addChildSubscriptionFromThis();
        return subs;
    }
private:
    KeySelector keySelector;
    ValueSelector valueSelector;
    ValuePrevSelector valuePrevSelector;
};

#endif // OPERATORTOMAP_HPP
