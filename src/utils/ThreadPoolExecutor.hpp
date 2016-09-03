#ifndef THREADPOOLEXECUTOR_HPP
#define THREADPOOLEXECUTOR_HPP

#include "Functions.hpp"
#include "MTQueue.hpp"
#include "Subscription.h"
#include <thread>
#include <vector>
#include <atomic>

template<typename T>
class ScheduledAction : public SubscriptionBase
{
public:
    ScheduledAction(T&& act) : action(std::move(act)), unsubscr(false)
    {}

    void operator()()
    {
        if(!unsubscr.load())
        {
            action();
        }
    }

    bool isUnsubscribe()
    {
        return unsubscr.load();
    }

    void unsubscribe()
    {
        unsubscr.store(true);
    }
private:
    T action;
    std::atomic<bool> unsubscr;
};

template<typename T>
using ScheduledActionPrtType = std::shared_ptr<ScheduledAction<T>>;

template<typename T>
class ThreadPoolExecutor
{
public:
    using ScheduledActionType = ScheduledActionPrtType<T>;

    ThreadPoolExecutor(size_t size) : done(false)
    {
        for(size_t i = 0; i < size; ++i)
        {
            workers.push_back(std::thread(&ThreadPoolExecutor::run, this));
        }
    }

    ThreadPoolExecutor(ThreadPoolExecutor&&) = default;
    ThreadPoolExecutor& operator = (ThreadPoolExecutor&&) = default;

    ThreadPoolExecutor(const ThreadPoolExecutor&) = delete;
    ThreadPoolExecutor& operator = (const ThreadPoolExecutor&) = delete;

    virtual ~ThreadPoolExecutor()
    {
        done.store(true);
        for(size_t i = 0; i < workers.size(); ++i)
        {
            if(workers[i].joinable())
            {
                workers[i].join();
            }
        }
    }

    ScheduledActionType submit(T&& action)
    {
        auto schedAction = std::make_shared<ScheduledAction<T>>(std::move(action));
        actions.push(schedAction);
        return schedAction;
    }

    void shutdown()
    {
        done.store(true);
    }
private:
    virtual void run()
    {
        while(!done.load())
        {
            ScheduledActionType action;
            if(actions.waitForAndPop(action, std::chrono::seconds(TIMEOUT)))
            {
                (*action)();
            }
        }
    }

    std::atomic<bool> done;
    MTQueue<ScheduledActionType> actions;
    std::vector<std::thread> workers;
    const int TIMEOUT = 2;
};
#endif // THREADPOOLEXECUTOR_HPP
