#include "ThreadPool.hpp"

namespace Core {

ThreadPool::ThreadPool(size_t numThreads) : _stop(false), _busyThreads(0) {
    for(size_t i = 0; i < numThreads; ++i) {
        _workers.emplace_back([this] {
            for(;;) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->_queueMutex);
                    this->_condition.wait(lock, [this]{ return this->_stop || !this->_tasks.empty(); });
                    
                    if(this->_stop && this->_tasks.empty())
                        return;
                    
                    task = std::move(this->_tasks.front());
                    this->_tasks.pop();
                }

                this->_busyThreads++;
                task();
                this->_busyThreads--;
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        _stop = true;
    }
    _condition.notify_all();
    for(std::thread &worker: _workers) {
        if(worker.joinable())
            worker.join();
    }
}

size_t ThreadPool::getBusyCount() const {
    return _busyThreads;
}

size_t ThreadPool::getThreadCount() const {
    return _workers.size();
}

} // namespace Core
