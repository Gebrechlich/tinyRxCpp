#ifndef OPERATORTAKE
#define OPERATORTAKE

#include "Operator.hpp"
#include <iostream>
#include <memory>
template<typename T>
class OperatorTake : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    struct TakeSubscriber : public CompositeSubscriber<T,T>
    {
        TakeSubscriber(ThisSubscriberType p, size_t i) :
            CompositeSubscriber<T,T>(p), index(i)
        {
        }

        void onNext(const T& t) override
        {
            if(!this->isUnsubscribe() && currentIndex < index)
            {
                ++currentIndex;
                complete = currentIndex == index;
                this->child->onNext(t);
            }

            if(complete)
            {
                this->child->onComplete();
                this->unsubscribe();
            }
        }

        size_t index;
        size_t currentIndex = 0;
        bool complete = false;
    };

public:
    OperatorTake(size_t i) : Operator<T,T>(),
        index(i)
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<TakeSubscriber>(t, index);
        subs->addChildSubscriptionFromThis();
        return subs;
    }
private:
    size_t index;
};

#endif // OPERATORTAKE

