#ifndef MTQUEUE
#define MTQUEUE
#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class MTQueue
{
public:
    MTQueue()
    {}

    MTQueue(const MTQueue& o)
    {
        std::lock_guard<std::mutex> lk(o.mut);
        data_queue = o.data_queue;
    }

    MTQueue& operator=(const MTQueue&) = delete;

    void setLimit(size_t l)
    {
        limit = l;
    }

    void push(const T& dat)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(dat);
        cond.notify_one();
    }

    bool offer(const T& dat)
    {
        size_t s = size();
        if(s >= limit)
        {
            return false;
        }
        push(dat);
        return true;
    }

    bool tryPop(T& value)
    {
        std::lock_guard<std::mutex> ul(mut);
        if(data_queue.empty())
        {
            return false;
        }
        value = data_queue.front();
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> tryPop()
    {
        std::lock_guard<std::mutex> ul(mut);
        if(data_queue.empty())
        {
            return std::make_shared<T>();
        }
        std::shared_ptr<T>res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    void waitAndPop(T& value)
    {
        std::unique_lock<std::mutex> ul(mut);
        cond.wait(ul,[&]{return !data_queue.empty();});
        value = data_queue.front();
        data_queue.pop();
    }

    std::shared_ptr<T> waitAndPop()
    {
        std::unique_lock<std::mutex> ul(mut);
        cond.wait(ul,[&]{return !data_queue.empty();});
        std::shared_ptr<T>res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    template<typename Rep, typename Period>
    bool waitForAndPop(T& value, const std::chrono::duration<Rep, Period>& timeout)
    {
        bool result = false;
        std::unique_lock<std::mutex> ul(mut);
        cond.wait_for(ul,timeout,[&]{return !data_queue.empty();});
        if(!data_queue.empty())
        {
            value = data_queue.front();
            data_queue.pop();
            result = true;
        }
        return result;
    }

    template<typename Rep, typename Period>
    std::shared_ptr<T> waitForAndPop(const std::chrono::duration<Rep, Period>& timeout)
    {
        std::unique_lock<std::mutex> ul(mut);
        cond.wait_for(ul,timeout,[&]{return !data_queue.empty();});
        std::shared_ptr<T>res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> ul(mut);
        return data_queue.empty();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> ul(mut);
        return data_queue.size();
    }

    void clear()
    {
        std::lock_guard<std::mutex> ul(mut);
        std::queue<T> empty;
        std::swap(data_queue, empty);
    }

private:
    mutable std::mutex mut;
    std::condition_variable cond;
    std::queue<T> data_queue;
    size_t limit = sizeof(size_t);
};

#endif // MTQUEUE

