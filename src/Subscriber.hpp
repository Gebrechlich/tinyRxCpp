#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H
#include "Observer.hpp"
#include "SubscriptionsList.hpp"
#include "Producer.hpp"

template<typename T>
class Subscriber : public Observer<T>, public SubscriptionBase
{
public:
    Subscriber() : Observer<T>(){}

    Subscriber(typename Observer<T>::ThisOnNextFP nfp,
               typename Observer<T>::ThisOnErrorFP efp,
               typename Observer<T>::ThisOnCompleteFP cfp) : Observer<T>(nfp, efp, cfp)
    {}

    bool isUnsubscribe() override
    {
        return subscriptions.isUnsubscribe();
    }

    void unsubscribe() override
    {
        subscriptions.unsubscribe();
    }

    void add(const SharedSubscription& subscription)
    {
        subscriptions.add(subscription);
    }

    virtual void onStart()
    {}

    void setProducer(const ProducerRef& p)
    {
        std::lock_guard<std::mutex> lk(lockMutex);
        producer = p;
    }

    void request(unsigned int n)
    {
        std::lock_guard<std::mutex> lk(lockMutex);
        if(producer)
        {
            producer->request(n);
        }
    }
protected:
    SubscriptionsList subscriptions;
    ProducerRef producer;
    std::mutex lockMutex;
};

template<typename T, typename U>
class CompositeSubscriber : public Subscriber<T>
{
public:
    using ChildSubscriberType = std::shared_ptr<Subscriber<U>>;

    CompositeSubscriber(ChildSubscriberType ptr) : Subscriber<T>(), child(std::move(ptr))
    {}

    void onError(std::exception_ptr ex) override
    {
        child->onError(ex);
    }

    void onComplete() override
    {
        child->onComplete();
    }

    virtual void addChildSubscriptionFromThis()
    {
        child->add(this->shared_from_this());
    }
protected:
    ChildSubscriberType child;
};

#endif // SUBSCRIBER_H
