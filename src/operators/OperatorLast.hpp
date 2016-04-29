#ifndef OPERATORLAST
#define OPERATORLAST
#include "Operator.hpp"

template<typename T>
class OperatorLast : public Operator<T, T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    struct LastSubscriber : public CompositeSubscriber<T,T>
    {
        LastSubscriber(ThisSubscriberType p) :
            CompositeSubscriber<T,T>(p)
        {}

        void onNext(const T& t) override
        {
            last = t;
        }

        void onComplete() override
        {
            this->child->onNext(last);
            this->child->onComplete();
        }

        T last;
    };

public:
    OperatorLast() : Operator<T, T>()
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<LastSubscriber>(t);
        subs->addChildSubscriptionFromThis();
        return subs;
    }

};

#endif // OPERATORLAST

