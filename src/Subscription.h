#ifndef SUBSCRIPTION
#define SUBSCRIPTION
#include <memory>
#include <mutex>
#include "Util.hpp"

struct SubscriptionBase : std::enable_shared_from_this<SubscriptionBase>
{
    virtual ~SubscriptionBase() = default;
    virtual bool isUnsubscribe() = 0;
    virtual void unsubscribe() = 0;
};

using SubscriptionPtrType = std::shared_ptr<SubscriptionBase>;

class WeekSubscription : public SubscriptionBase
{
public:
    WeekSubscription(std::weak_ptr<SubscriptionBase>&& ptr) :
        mut(make_unique<std::mutex>()), subscriptionPtr(std::move(ptr)){}

    WeekSubscription(WeekSubscription&&) = default;
    WeekSubscription(const WeekSubscription&) = delete;

    bool isUnsubscribe() override
    {
        bool res = true;
        (*mut).lock();
        if(auto spt = subscriptionPtr.lock())
        {
            res = spt->isUnsubscribe();
        }
        (*mut).unlock();
        return res;
    }

    void unsubscribe() override
    {
        (*mut).lock();
        if(auto spt = subscriptionPtr.lock())
        {
            spt->unsubscribe();
        }
        (*mut).unlock();
    }
private:
    std::unique_ptr<std::mutex> mut;
    std::weak_ptr<SubscriptionBase> subscriptionPtr;
};

class SharedSubscription : public SubscriptionBase
{
public:
    SharedSubscription(){}

    SharedSubscription(std::shared_ptr<SubscriptionBase>&& ptr) :
        subscriptionPtr(std::move(ptr)){}

    SharedSubscription(const std::shared_ptr<SubscriptionBase>& ptr) :
        subscriptionPtr(ptr){}

    bool isUnsubscribe() override
    {
        if(subscriptionPtr)
        {
            return subscriptionPtr->isUnsubscribe();
        }
        return true;
    }

    void unsubscribe() override
    {
        if(subscriptionPtr)
        {
            subscriptionPtr->unsubscribe();
        }
    }
private:
    std::shared_ptr<SubscriptionBase> subscriptionPtr;
};

#endif // SUBSCRIPTION

