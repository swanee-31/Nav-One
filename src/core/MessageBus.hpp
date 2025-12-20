#pragma once

#include "core/NavData.hpp"
#include <functional>
#include <vector>
#include <mutex>
#include <map>
#include <memory>

namespace Core {

class MessageBus {
public:
    using ListenerId = size_t;
    using Callback = std::function<void(const NavData&)>;

    static MessageBus& instance() {
        static MessageBus instance;
        return instance;
    }

    ListenerId subscribe(Callback callback) {
        std::lock_guard<std::mutex> lock(mutex);
        ListenerId id = nextId++;
        listeners[id] = callback;
        return id;
    }

    void unsubscribe(ListenerId id) {
        std::lock_guard<std::mutex> lock(mutex);
        listeners.erase(id);
    }

    void publish(const NavData& data) {
        std::lock_guard<std::mutex> lock(mutex);
        for (const auto& [id, callback] : listeners) {
            callback(data);
        }
    }

private:
    MessageBus() = default;
    ~MessageBus() = default;
    MessageBus(const MessageBus&) = delete;
    MessageBus& operator=(const MessageBus&) = delete;

    std::mutex mutex;
    std::map<ListenerId, Callback> listeners;
    ListenerId nextId = 0;
};

} // namespace Core
