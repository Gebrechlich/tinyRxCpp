#ifndef SUBSCRIPTION
#define SUBSCRIPTION
#include <memory>
#include <mutex>
#include <vector>
#include "utils/Util.hpp"

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

class SubscriptionsList : public SubscriptionBase
{
public:
    void add(const SubscriptionPtrType& subscription)
    {
        subscriptions.push_back(subscription);
    }

    bool isUnsubscribe()
    {
        return unsubscr;
    }

    void unsubscribe()
    {
        if(!unsubscr)
        {
            std::lock_guard<std::mutex> l(lockMutex);
            for(auto s : subscriptions)
            {
                if(s != nullptr && !s->isUnsubscribe())
                {
                    s->unsubscribe();
                }
            }
            unsubscr = true;
        }
    }
private:
    std::vector<SubscriptionPtrType> subscriptions;
    std::mutex lockMutex;
    volatile bool unsubscr = false;
};

#endif // SUBSCRIPTION

