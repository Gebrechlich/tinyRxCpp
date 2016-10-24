#ifndef OPERATORSCAN
#define OPERATORSCAN

#include "Operator.hpp"
#include "../Functions.hpp"
#include <type_traits>
#include <utility>

template<typename T, typename Accumulator>
class OperatorScan : public Operator<T, typename std::result_of<Accumulator(const T&, const T&)>::type>
{
    using ResultType           = typename std::result_of<Accumulator(const T&, const T&)>::type;
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType   = typename CompositeSubscriber<T, ResultType>::ChildSubscriberType;
    using AccumulatorType      = typename std::decay<Accumulator>::type;

    struct ScanSubscriber : public CompositeSubscriber<T, ResultType>
    {
        ScanSubscriber(ThisSubscriberType p, AccumulatorType&& accumulator, T&& seed, bool useSeed) :
            CompositeSubscriber<T, ResultType>(p), accumulator(std::move(accumulator)),
            result(seed), useSeed(useSeed)
        {}

        void onNext(const T& t) override
        {
            if(!once && !useSeed)
            {
                result = t;
                once = true;
            }
            else
            {
                ResultType v = accumulator(result, t);
                result = v;
            }
            this->child->onNext(result);
        }

        AccumulatorType accumulator;
        ResultType result;
        bool useSeed;
        bool once = false;
    };

public:
    OperatorScan(const AccumulatorType& accumulator, T seed = T(), bool useSeed = false) : Operator<T,ResultType>(),
        accumulator(accumulator), seed(std::move(seed)), useSeed(useSeed)
    {}

    OperatorScan(AccumulatorType&& accumulator, T seed = T(), bool useSeed = false) : Operator<T,ResultType>(),
        accumulator(std::move(accumulator)), seed(std::move(seed)), useSeed(useSeed)
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<ScanSubscriber>(t, std::move(accumulator), std::move(seed), useSeed);
        subs->addChildSubscriptionFromThis();
        return subs;
    }
private:
    AccumulatorType accumulator;
    T seed;
    bool useSeed;
};


#endif // OPERATORSCAN

