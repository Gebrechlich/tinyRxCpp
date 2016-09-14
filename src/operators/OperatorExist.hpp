#ifndef OPERATOREXIST
#define OPERATOREXIS
#include "Operator.hpp"
#include <type_traits>

template<typename T, typename Predicate>
class OperatorExist : public Operator<T,bool>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType   = std::shared_ptr<Subscriber<bool>>;
    using PredicateType        = typename std::decay<Predicate>::type;

    struct ExistSubscriber : public Subscriber<T>
    {
        ExistSubscriber(ThisSubscriberType p, PredicateType&& pred) :
            child(p), predicate(std::move(pred))
        {}

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
        PredicateType predicate;
        bool done = false;
    };

public:
    OperatorExist(const PredicateType& pred) : Operator<T,bool>(),
        predicate(pred)
    {}

    OperatorExist(PredicateType&& pred) : Operator<T,bool>(),
        predicate(std::move(pred))
    {}

    virtual SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        return std::make_shared<ExistSubscriber>(t, std::move(predicate));
    }
private:
    PredicateType predicate;
};

#endif // OPERATOREXIST

