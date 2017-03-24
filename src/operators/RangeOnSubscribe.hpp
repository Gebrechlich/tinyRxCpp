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
        T countInner(count);
        for(T i = start; countInner > 0 && !t->isUnsubscribe(); ++i, --countInner)
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
