#ifndef OPERATOREXIST
#define OPERATOREXIS
#include "Operator.hpp"

template<typename T>
class OperatorExist : public Operator<T,bool>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = std::shared_ptr<Subscriber<bool>>;

    struct ExistSubscriber : public Subscriber<T>
    {
        ExistSubscriber(ThisSubscriberType p, Predicat<T> pred) :
            child(p), predicate(std::move(pred))
        {
        }

        void onNext(const T& t) override
        {
            if(predicate(t) && !done)
            {
                done = true;
                child->onNext(true);
            }
        }

        void onError(std::exception_ptr ex) override
        {
            child->onError(ex);
        }

        void onComplete() override
        {
            if(!done)
            {
                child->onNext(false);
                done = true;
            }
            child->onComplete();
        }

        ThisSubscriberType child;
        Predicat<T> predicate;
        bool done = false;
    };

public:
    OperatorExist(Predicat<T> pred) : Operator<T,bool>(),
        predicate(std::move(pred))
    {}

    virtual SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        return std::make_shared<ExistSubscriber>(t, std::move(predicate));
    }
private:
    Predicat<T> predicate;
};

#endif // OPERATOREXIST

