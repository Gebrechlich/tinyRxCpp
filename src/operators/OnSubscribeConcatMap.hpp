#ifndef ONSUBSCRIBECONCATMAP_HPP
#define ONSUBSCRIBECONCATMAP_HPP

#include "OnSubscribeBase.hpp"
#include <atomic>
#include <queue>
#include <type_traits>
#include <vector>

template<typename T, typename R, typename Mapper>
class OnSubscribeConcatMap : public OnSubscribeBase<R>
{
public:
    using OnSubscribePtrType      = std::shared_ptr<OnSubscribeBase<T>>;
    using MapperType              = typename std::decay<Mapper>::type;
    using ThisChildSubscriberType = typename CompositeSubscriber<T,R>::ChildSubscriberType;
    using MapObservableType       = typename std::result_of<MapperType(const T&)>::type;

    struct InnerConcatMapSubscriber;

    struct ConcatMapSubscriber : public CompositeSubscriber<T,R>
    {
        ConcatMapSubscriber(ThisChildSubscriberType child,const MapperType& mapper) :
            CompositeSubscriber<T,R>(child), mapper(mapper), requested(0)
        {}

        void onNext(const T& t) override
        {
            ++requested;
            if(!this->isUnsubscribe())
            {
                extQueue.push(t);
                process();
            }
        }

        void onNextInner(const R& t)
        {
            this->child->onNext(t);
        }

        void onError(std::exception_ptr ex) override
        {
            done = true;
            this->child->onError(ex);
        }

        void onErrorInner(std::exception_ptr ex)
        {
            onError(ex);
            this->unsubscribe();
        }

        void onCompleteInner()
        {
            --requested;
            if(parentComplete && !done && requested.load() == 0)
            {
                done = true;
                this->child->onComplete();
            }
            active = false;
        }

        void onComplete() override
        {
            parentComplete = true;
            if(requested.load() == 0 && !done)
            {
                done = true;
                this->child->onComplete();
            }
        }

        void process()
        {
            while(true)
            {
                if(this->isUnsubscribe())
                {
                    return;
                }

                if(!active)
                {
                    if(extQueue.empty())
                    {
                        return;
                    }
                    auto o = extQueue.front();
                    extQueue.pop();
                    auto obs = std::make_shared<MapObservableType>(std::move(mapper(o)));
                    std::shared_ptr<Subscriber<R>> innerSubscriber = std::make_shared<InnerConcatMapSubscriber>
                            (std::dynamic_pointer_cast<ConcatMapSubscriber>(this->shared_from_this()));
                    active = true;
                    obs->subscribe(innerSubscriber);
                    addObservableReference(obs);
                    this->add(innerSubscriber);
                }
            }
        }

        void addObservableReference(const std::shared_ptr<MapObservableType>& obs)
        {
            pool.push_back(obs);
        }

        MapperType mapper;
        std::queue<T> extQueue;
        std::atomic_int requested;
        volatile bool parentComplete = false;
        volatile bool done = false;
        volatile bool active = false;
        //Keep observable references
        std::vector<std::shared_ptr<MapObservableType>> pool;
    };

    struct InnerConcatMapSubscriber : public Subscriber<R>
    {
        InnerConcatMapSubscriber(std::shared_ptr<ConcatMapSubscriber> child) : child(child)
        {}

        void onNext(const R& t) override
        {
            child->onNextInner(t);
        }

        void onError(std::exception_ptr ex) override
        {
            child->onErrorInner(ex);
        }

        void onComplete() override
        {
            child->onCompleteInner();
        }

        std::shared_ptr<ConcatMapSubscriber> child;
    };


    OnSubscribeConcatMap(OnSubscribePtrType source, const MapperType& mapper) : source(source)
      ,mapper(mapper)
    {}

    OnSubscribeConcatMap(OnSubscribePtrType source, MapperType&& mapper) : source(source)
      ,mapper(std::move(mapper))
    {}

    void operator()(const SubscriberPtrType<R>& s) override
    {
        if(s == nullptr)
        {
            return;
        }

        std::shared_ptr<ConcatMapSubscriber> parent = std::make_shared<ConcatMapSubscriber>(s, mapper);
        s->add(parent);

        if(!s->isUnsubscribe())
        {
            (*source)(parent);
        }
    }

private:
    OnSubscribePtrType source;
    MapperType mapper;
};

#endif // ONSUBSCRIBECONCATMAP_HPP
