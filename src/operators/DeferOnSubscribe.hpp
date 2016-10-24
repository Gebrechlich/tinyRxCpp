#ifndef DEFERONSUBSCRIBE_HPP
#define DEFERONSUBSCRIBE_HPP
#include "OnSubscribeBase.hpp"

template<typename T, typename ObservableFactory>
class DeferOnSubscribe : public OnSubscribeBase<T>
{
public:
    DeferOnSubscribe(const ObservableFactory& observFactory) :
        observableFactory(observFactory)
    {}

    DeferOnSubscribe(ObservableFactory&& observFactory) :
        observableFactory(std::move(observFactory))
    {}

    void operator()(const SubscriberPtrType<T>& t) override
    {
        auto o = observableFactory();
        o.subscribe(t);
    }
private:
    ObservableFactory observableFactory;
};

#endif // DEFERONSUBSCRIBE_HPP
