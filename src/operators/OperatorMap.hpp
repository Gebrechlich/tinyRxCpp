#ifndef OPERATORMAP_H
#define OPERATORMAP_H
#include "Operator.hpp"
#include <type_traits>

template<typename T, typename Mapper>
class OperatorMap : public Operator<T, typename std::result_of<Mapper(const T&)>::type>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using MapResultType        = typename std::result_of<Mapper(const T&)>::type;
    using ThisSubscriberType   = typename CompositeSubscriber<T,MapResultType>::ChildSubscriberType;
    using MapFunctionType      = typename std::decay<Mapper>::type;

    struct MapSubscriber : public CompositeSubscriber<T,MapResultType>
    {
        MapSubscriber(ThisSubscriberType p, MapFunctionType&& f) :
            CompositeSubscriber<T,MapResultType>(p), func(std::move(f))
        {
        }

        void onNext(const T& t) override
        {
            this->child->onNext(func(t));
        }

        MapFunctionType func;
    };

public:
    OperatorMap(){}

    OperatorMap(const MapFunctionType& f) : Operator<T, MapResultType>(),
        func(f)
    {}

    OperatorMap(MapFunctionType&& f) : Operator<T, MapResultType>(),
        func(std::move(f))
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<MapSubscriber>(t, std::move(func));
        subs->addChildSubscriptionFromThis();
        return subs;
    }
private:
    MapFunctionType func;
};

#endif // OPERATORMAP_H
