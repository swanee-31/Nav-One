#pragma once

#include "IService.hpp"
#include <asio.hpp>
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>
#include <vector>

namespace Network {

class UdpService : public IService {
public:
    using DataCallback = std::function<void(const std::vector<char>&, const std::string&)>;

    UdpService(int port, DataCallback callback);
    ~UdpService();

    void start() override;
    void stop() override;
    bool isRunning() const override { return running; }

private:
    void startReceive();
    void handleReceive(const std::error_code& error, std::size_t bytes_transferred);

    int port;
    DataCallback onDataReceived;
    
    asio::io_context ioContext;
    std::unique_ptr<asio::ip::udp::socket> socket;
    asio::ip::udp::endpoint remoteEndpoint;
    std::vector<char> recvBuffer;
    
    std::thread serviceThread;
    std::atomic<bool> running{false};
};

} // namespace Network
