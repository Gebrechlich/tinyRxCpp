#ifndef OPERATORTOMAP_HPP
#define OPERATORTOMAP_HPP
#include "Operator.hpp"
#include <type_traits>
#include <map>

template<typename T, typename KeySelector, typename ValueSelector>
using MapT = std::map<typename std::result_of<KeySelector(const T&)>::type,
                      typename std::result_of<ValueSelector(const T&)>::type>;

template<typename T, typename KeySelector, typename ValueSelector, typename ValuePrevSelector>
class OperatorToMap : public Operator<T, MapT<T,KeySelector,ValueSelector>>
{
    using KeyType               = typename std::result_of<KeySelector(const T&)>::type;
    using ValueType             = typename std::result_of<ValueSelector(const T&)>::type;
    using SourceSubscriberType  = std::shared_ptr<Subscriber<T>>;
    using MapType               = MapT<T,KeySelector,ValueSelector>;
    using ThisSubscriberType    = typename CompositeSubscriber<T, MapType>::ChildSubscriberType;
    using KeySelectorType       = typename std::decay<KeySelector>::type;
    using ValueSelectorType     = typename std::decay<ValueSelector>::type;
    using ValuePrevSelectorType = typename std::decay<ValuePrevSelector>::type;

    struct ToMapSubscriber : public CompositeSubscriber<T, MapType>
    {
        ToMapSubscriber(ThisSubscriberType p, KeySelectorType&& kSelector, ValueSelectorType&& vSelector
                        ,ValuePrevSelectorType&& vpSelector) :
            CompositeSubscriber<T, MapType>(p), keySelector(std::move(kSelector)),
            valueSelector(std::move(vSelector)), valuePrevSelector(std::move(vpSelector))
        {}

        void onNext(const T& t) override
        {
            auto key = (*keySelector)(t);
            auto value = (*valueSelector)(t);
            auto prev = (map.find(key) == map.end() ? value : map[key]);
            map[key] = (*valuePrevSelector)(value, prev);
        }

        void onComplete() override
        {
            this->child->onNext(map);
            this->child->onComplete();
        }

        KeySelectorType keySelector;
        ValueSelectorType valueSelector;
        ValuePrevSelectorType valuePrevSelector;
        MapType map;
    };

public:
    OperatorToMap(){}
    OperatorToMap(KeySelectorType kSelector, ValueSelectorType vSelector, ValuePrevSelectorType vpSelector) : Operator<T, MapType>(),
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
    KeySelectorType keySelector;
    ValueSelectorType valueSelector;
    ValuePrevSelectorType valuePrevSelector;
};

#endif // OPERATORTOMAP_HPP
