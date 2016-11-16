#ifndef ONSUBSCRIBEFLATMAP_HPP
#define ONSUBSCRIBEFLATMAP_HPP
#include "OnSubscribeBase.hpp"
#include <type_traits>
#include <vector>


template<typename T, typename R, typename Mapper>
class OnSubscribeFlatMap : public OnSubscribeBase<R>
{
public:
    using OnSubscribePtrType      = std::shared_ptr<OnSubscribeBase<T>>;
    using MapperType              = typename std::decay<Mapper>::type;
    using ThisChildSubscriberType = typename CompositeSubscriber<T,R>::ChildSubscriberType;
    using MapObservableType       = typename std::result_of<MapperType(const T&)>::type;

    struct InnerFlatMapSubscriber;

    struct FlatMapSubscriber : public CompositeSubscriber<T,R>
    {
        FlatMapSubscriber(ThisChildSubscriberType child,const MapperType& mapper) :
            CompositeSubscriber<T,R>(child), mapper(std::move(mapper)), requested(0)
        {}

        void onNext(const T& t) override
        {
            ++requested;
            if(!this->isUnsubscribe())
            {
                auto obs = std::make_shared<MapObservableType>(std::move(mapper(t)));
                std::shared_ptr<Subscriber<R>> innerSubscriber = std::make_shared<InnerFlatMapSubscriber>
                        (std::dynamic_pointer_cast<FlatMapSubscriber>(this->shared_from_this()));

                obs->subscribe(innerSubscriber);
                addObservableReference(obs);
                this->add(innerSubscriber);
            }
        }

        void onNextInner(const R& t)
        {
            this->child->onNext(t);
        }

        void onErrorInner(std::exception_ptr ex)
        {
            this->child->onError(ex);
        }

        void onCompleteInner()
        {
            --requested;
            if(parentComplete && !done && requested.load() == 0)
            {
                done = true;
                this->child->onComplete();
            }
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

        void addObservableReference(const std::shared_ptr<MapObservableType>& obs)
        {
            pool.push_back(obs);
        }

        MapperType mapper;
        std::atomic_int requested;
        volatile bool parentComplete = false;
        volatile bool done = false;
        //Keep observable references
        std::vector<std::shared_ptr<MapObservableType>> pool;
    };

    struct InnerFlatMapSubscriber : public Subscriber<R>
    {
        InnerFlatMapSubscriber(std::shared_ptr<FlatMapSubscriber> child) : child(child)
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

        std::shared_ptr<FlatMapSubscriber> child;
    };

    OnSubscribeFlatMap(OnSubscribePtrType source, const MapperType& mapper) : source(source)
      ,mapper(mapper)
    {}

    OnSubscribeFlatMap(OnSubscribePtrType source, MapperType&& mapper) : source(source)
      ,mapper(std::move(mapper))
    {}

    void operator()(const SubscriberPtrType<R>& s) override
    {
        if(s == nullptr)
        {
            return;
        }

        std::shared_ptr<FlatMapSubscriber> parent = std::make_shared<FlatMapSubscriber>(s, mapper);
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


#endif // ONSUBSCRIBEFLATMAP_HPP
