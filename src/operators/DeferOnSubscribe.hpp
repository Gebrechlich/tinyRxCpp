#ifndef DEFERONSUBSCRIBE_HPP
#define DEFERONSUBSCRIBE_HPP
#include "OnSubscribeBase.hpp"
#include <vector>

template<typename T, typename ObservableFactory>
class DeferOnSubscribe : public OnSubscribeBase<T>
{
public:
    using ObservableType = typename std::result_of<ObservableFactory()>::type;
    using RefObservableType = typename std::shared_ptr<ObservableType>;

    DeferOnSubscribe(const ObservableFactory& observFactory) :
        observableFactory(observFactory)
    {}

    DeferOnSubscribe(ObservableFactory&& observFactory) :
        observableFactory(std::move(observFactory))
    {}

    void operator()(const SubscriberPtrType<T>& t) override
    {
        RefObservableType o = std::make_shared<ObservableType>(std::move(observableFactory()));
        o->subscribe(t);
        pool.push_back(o);
    }
private:
    ObservableFactory observableFactory;
    //Keep observable references
    std::vector<RefObservableType> pool;
};

#endif // DEFERONSUBSCRIBE_HPP
