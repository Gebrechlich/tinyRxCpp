#ifndef OBSERVER_H
#define OBSERVER_H
#include "Functions.hpp"
#include <exception>

template<typename T>
struct Observer
{
    using ThisOnCompleteFP = Action0_t;
    using ThisOnNextFP = Action1_t<T>;
    using ThisOnErrorFP = Action1_t<std::exception_ptr>;

    ThisOnNextFP onNextFp;
    ThisOnErrorFP onErrorFp;
    ThisOnCompleteFP onCompleteFp;

    Observer() = default;
    virtual ~Observer() = default;
    Observer(Observer&&) = default;
    Observer& operator = (Observer&&) = default;

    Observer(ThisOnNextFP nfp, ThisOnErrorFP efp, ThisOnCompleteFP cfp) :
        onNextFp(nfp), onErrorFp(efp), onCompleteFp(cfp)
    {}

    virtual void onNext(const T& t)
    {
        if(onNextFp)
        {
            onNextFp(t);
        }
    }

    virtual void onError(std::exception_ptr ex)
    {
        if(onErrorFp)
        {
            onErrorFp(ex);
        }
        else
        {
            std::rethrow_exception(ex);
        }
    }

    virtual void onComplete()
    {
        if(onCompleteFp)
        {
            onCompleteFp();
        }
    }

};

#endif // OBSERVER_H
