#ifndef RANGEONSUBSCRIBE_HPP
#define RANGEONSUBSCRIBE_HPP
#include "OnSubscribeBase.hpp"

template<typename T>
class RangeOnSubscribe : public OnSubscribeBase<T>
{
public:
    RangeOnSubscribe(T start, T count) : start(start), count(count)
    {}

    void operator()(const SubscriberPtrType<T>& t) override
    {
        for(T i = start; i < count && !t->isUnsubscribe(); ++i)
        {
            t->onNext(i);
        }
        t->onComplete();
    }

private:
    T start;
    T count;
};

#endif // RANGEONSUBSCRIBE_HPP
