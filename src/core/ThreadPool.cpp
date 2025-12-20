#include "ThreadPool.hpp"

namespace Core {

ThreadPool::ThreadPool(size_t numThreads) : stop(false), busyThreads(0) {
    for(size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            for(;;) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });
                    
                    if(this->stop && this->tasks.empty())
                        return;
                    
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                this->busyThreads++;
                task();
                this->busyThreads--;
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers) {
        if(worker.joinable())
            worker.join();
    }
}

size_t ThreadPool::getBusyCount() const {
    return busyThreads;
}

size_t ThreadPool::getThreadCount() const {
    return workers.size();
}

} // namespace Core
