#ifndef THREADPOOLEXECUTOR_HPP
#define THREADPOOLEXECUTOR_HPP

#include "../Functions.hpp"
#include "MTQueue.hpp"
#include "../Subscription.hpp"
#include <thread>
#include <vector>
#include <atomic>

class ThreadPoolExecutor
{
public:
    using ScheduledActionType = ActionRefType;

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
        shutdown();
        submitLock.lock();

        for(size_t i = 0; i < workers.size(); ++i)
        {
            if(workers[i].joinable())
            {
                if(actions.size() > 0)
                {
                    workers[i].join();
                }
                else
                {
                    workers[i].detach();
                }
            }
        }

        submitLock.unlock();
    }

    void submit(ScheduledActionType action)
    {
        submitLock.lock();
        actions.push(action);
        submitLock.unlock();
    }

    void shutdown()
    {
        done.store(true);
    }

private:

    virtual void run()
    {
        while(true)
        {
            ScheduledActionType action;
            if(actions.waitForAndPop(action, std::chrono::seconds(TIMEOUT)))
            {
                (*action)();
            }
            bool isDone = done.load();
            if(isDone)
            {
                return;
            }
        }
    }
    std::mutex submitLock;
    std::atomic<bool> done;
    MTQueue<ScheduledActionType> actions;
    std::vector<std::thread> workers;
    const int TIMEOUT = 2;
};
#endif // THREADPOOLEXECUTOR_HPP
