#ifndef SUBSCRIPTIONSLIST
#define SUBSCRIPTIONSLIST
#include "Subscription.h"
#include <vector>
#include <mutex>

class SubscriptionsList : public SubscriptionBase
{
public:
    void add(const SharedSubscription& subscription)
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
            for(SharedSubscription s : subscriptions)
            {
                if(!s.isUnsubscribe())
                {
                    s.unsubscribe();
                }
            }
            unsubscr = true;
        }
    }
private:
    std::vector<SharedSubscription> subscriptions;
    std::mutex lockMutex;
    volatile bool unsubscr = false;
};

#endif // SUBSCRIPTIONSLIST

