#ifndef MTQUEUE
#define MTQUEUE
#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class MTQueue
{
public:
    void push(const T& dat)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(dat);
        cond.notify_one();
    }

    T waitAndPop()
    {
        std::unique_lock<std::mutex> ul(mut);
        cond.wait(ul,[&]{return !data_queue.empty();});
        T dat = std::move(data_queue.front());
        data_queue.pop();
        return dat;
    }
private:
    std::mutex mut;
    std::condition_variable cond;
    std::queue<T> data_queue;
};

#endif // MTQUEUE

