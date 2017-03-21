#ifndef OPERATORSYNCHRONIZE_HPP
#define OPERATORSYNCHRONIZE_HPP

#include "Operator.hpp"
#include <mutex>

template<typename T, typename L>
class OperatorSynchronize : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType   = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    struct SynchronizeSubscriber : public CompositeSubscriber<T,T>
    {
        SynchronizeSubscriber(ThisSubscriberType p) :
            CompositeSubscriber<T,T>(p)
        {}

        void onNext(const T& t) override
        {
            std::lock_guard<L> ul(lock);
            this->child->onNext(t);
        }

        void onComplete() override
        {
            std::lock_guard<L> ul(lock);
            this->child->onComplete();
        }

        L lock;
    };

public:

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<SynchronizeSubscriber>(t);
        subs->addChildSubscriptionFromThis();
        return subs;
    }
};


#endif // OPERATORSYNCHRONIZE_HPP
