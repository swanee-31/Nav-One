#pragma once
#include <string>

namespace Network {

class IService {
public:
    virtual ~IService() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual void send(const std::string& data) {}
};

} // namespace Network
