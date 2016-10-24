#ifndef ONSUBSCRIBEBASE_HPP
#define ONSUBSCRIBEBASE_HPP
#include "../Functions.hpp"
#include "../Subscriber.hpp"

template<typename T>
using SubscriberPtrType = std::shared_ptr<Subscriber<T>>;

template<typename R>
class OnSubscribeBase : public Action1<SubscriberPtrType<R>>
{
public:
    OnSubscribeBase() : Action1<SubscriberPtrType<R>>(){}

    OnSubscribeBase(typename Action1<SubscriberPtrType<R>>::ActionFp fp) :
        Action1<SubscriberPtrType<R>>(fp){}
};

#endif // ONSUBSCRIBEBASE_HPP
