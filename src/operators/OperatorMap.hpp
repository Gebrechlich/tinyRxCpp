#ifndef OPERATORMAP_H
#define OPERATORMAP_H
#include "Operator.hpp"

template<typename T, typename R>
class OperatorMap : public Operator<T, R>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,R>::ChildSubscriberType;
    using MapFunctionType = std::unique_ptr<Function1<R, T>>;

    struct MapSubscriber : public CompositeSubscriber<T,R>
    {
        MapSubscriber(ThisSubscriberType p, MapFunctionType f) :
            CompositeSubscriber<T,R>(p), func(std::move(f))
        {
        }

        void onNext(const T& t) override
        {
            this->child->onNext((*func)(t));
        }

        MapFunctionType func;
    };

public:
    OperatorMap(){}
    OperatorMap(MapFunctionType f) : Operator<T, R>(),
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
