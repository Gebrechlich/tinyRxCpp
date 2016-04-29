#ifndef OPERATORALL_HPP
#define OPERATORALL_HPP
#include "Operator.hpp"

template<typename T>
class OperatorAll : public Operator<T,bool>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,bool>::ChildSubscriberType;

    struct AllSubscriber : public CompositeSubscriber<T,bool>
    {
        AllSubscriber(ThisSubscriberType p, Predicat<T> pred) :
            CompositeSubscriber<T,bool>(p), predicate(std::move(pred))
        {
        }

        void onNext(const T& t) override
        {
            if(!predicate(t) && !done)
            {
                done = true;
                this->child->onNext(false);
            }
        }

        void onComplete() override
        {
            this->child->onComplete();
            if(!done)
            {
                this->child->onNext(true);
                done = true;
            }
        }

        Predicat<T> predicate;
        bool done = false;
    };

public:
    OperatorAll(Predicat<T> pred) : Operator<T,bool>(),
        predicate(std::move(pred))
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<AllSubscriber>(t, std::move(predicate));
        subs->addChildSubscriptionFromThis();
        return subs;
    }
private:
    Predicat<T> predicate;
};

#endif // OPERATORALL_HPP

