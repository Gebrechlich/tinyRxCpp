#ifndef LIFTONSUBSCRIBE_HPP
#define LIFTONSUBSCRIBE_HPP
#include "OnSubscribeBase.hpp"

template<typename A, typename B>
class LiftOnSubscribe : public OnSubscribeBase<A>
{
public:
    LiftOnSubscribe(std::shared_ptr<OnSubscribeBase<B>> parent, std::unique_ptr<Operator<B, A>> o) :
                    OnSubscribeBase<A>(), parentOnSubscribe(parent), op(std::move(o))
    {
    }

    void operator()(const SubscriberPtrType<A>& t) override
    {
        SubscriberPtrType<B> st = (*op)(t);
        (*parentOnSubscribe)(std::move(st));
    }

protected:
    std::shared_ptr<OnSubscribeBase<B>> parentOnSubscribe;
    std::unique_ptr<Operator<B, A>> op;
};

#endif // LIFTONSUBSCRIBE_HPP
