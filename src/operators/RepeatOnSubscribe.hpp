#ifndef REPEATONSUBSCRIBE_HPP
#define REPEATONSUBSCRIBE_HPP
#include "OnSubscribeBase.hpp"
#include <atomic>

template<typename T>
class RepeatOnSubscribe : public OnSubscribeBase<T>
{
public:
    using OnSubscribePtrType = std::shared_ptr<OnSubscribeBase<T>>;

    RepeatOnSubscribe(const OnSubscribePtrType& source, size_t count) :
    source(source), count(count), infinitely(!count)
    {}

    struct InnerSubscriber : public CompositeSubscriber<T,T>
    {
        using ThisChildSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

        InnerSubscriber(ThisChildSubscriberType child, std::atomic<size_t>& count) :
            CompositeSubscriber<T,T>(child), countRef(count), infinitely(!count)
        {}

        void onNext(const T& t) override
        {
            if(!this->child->isUnsubscribe())
            {
                this->child->onNext(t);
            }
        }

        void onComplete() override
        {
            if(!infinitely && countRef.load() == 0)
            {
                this->child->onComplete();
            }
        }

        void onError(std::exception_ptr ex) override
        {
            this->child->onError(ex);
            countRef.store(0);
        }

        std::atomic<size_t>& countRef;
        bool infinitely;
    };

    void operator()(const SubscriberPtrType<T>& subscriber) override
    {
        if(subscriber == nullptr)
        {
            return;
        }

        auto innerSubscriber = std::make_shared<InnerSubscriber>(subscriber, count);
        subscriber->add(SharedSubscription(innerSubscriber));

        while((infinitely || count--) && !subscriber->isUnsubscribe())
        {
            (*source)(innerSubscriber);
        }
    }
private:
    OnSubscribePtrType source;
    std::atomic<size_t> count;
    bool infinitely;
};

#endif // REPEATONSUBSCRIBE_HPP
