#ifndef OPERATORSCAN
#define OPERATORSCAN

#include "Operator.hpp"
#include "Functions.hpp"
#include <utility>

template<typename T, typename R>
class OperatorScan : public Operator<T, R>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,R>::ChildSubscriberType;
    using AccumulatorType = std::unique_ptr<Function2<R,T,T>>;

    struct ScanSubscriber : public CompositeSubscriber<T,R>
    {
        ScanSubscriber(ThisSubscriberType p,
                       AccumulatorType accumulator) :
            CompositeSubscriber<T,R>(p), accumulator(std::move(accumulator))
        {
        }

        void onNext(const T& t) override
        {
            if(!once)
            {
                result = static_cast<R>(t);
                once = true;
            }
            else
            {
                R v = (*accumulator)(result, t);
                result = v;
            }
            this->child->onNext(result);
        }

        AccumulatorType accumulator;
        bool once = false;
        R result;
    };

    struct ScanSeedSubscriber : public ScanSubscriber
    {
        ScanSeedSubscriber(ThisSubscriberType p, AccumulatorType accumulator,
                           Function0UniquePtr<R>&& seed) :
            ScanSubscriber(p, std::move(accumulator)), seedScan(std::move(seed))
        {
            result = (*seedScan)();
        }

        void onNext(const T& t) override
        {
            R v = (*(this->accumulator))(result, t);
            result = v;
            this->child->onNext(result);
        }

        R result;
        Function0UniquePtr<R> seedScan;
    };

public:
    OperatorScan(AccumulatorType accumulator) : Operator<T,R>(),
        accumulator(std::move(accumulator)), seed(nullptr)
    {}

    OperatorScan(AccumulatorType accumulator, Function0UniquePtr<R>&& seed) : Operator<T,R>(),
        accumulator(std::move(accumulator)), seed(std::move(seed))
    {
    }

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        if(!seed)
        {
            auto subs = std::make_shared<ScanSubscriber>(t, std::move(accumulator));
            subs->addChildSubscriptionFromThis();
            return subs;
        }
        else
        {
            auto subs = std::make_shared<ScanSeedSubscriber>(t, std::move(accumulator), std::move(seed));
            subs->addChildSubscriptionFromThis();
            return subs;
        }
    }
private:
    AccumulatorType accumulator;
    Function0UniquePtr<R> seed;
};


#endif // OPERATORSCAN

