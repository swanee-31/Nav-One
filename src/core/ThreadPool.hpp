#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

namespace Core {

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    // Enqueue a task to be executed by the thread pool
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

    // Get the number of busy threads (approximate)
    size_t getBusyCount() const;
    
    // Get total number of threads
    size_t getThreadCount() const;

private:
    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _tasks;
    
    std::mutex _queueMutex;
    std::condition_variable _condition;
    std::atomic<bool> _stop;
    std::atomic<size_t> _busyThreads;
};

// Template implementation
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(_queueMutex);

        // Don't allow enqueueing after stopping
        if(_stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        _tasks.emplace([task](){ (*task)(); });
    }
    _condition.notify_one();
    return res;
}

} // namespace Core
