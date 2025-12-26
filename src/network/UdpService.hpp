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
    bool isRunning() const override { return _running; }

private:
    void startReceive();
    void handleReceive(const std::error_code& error, std::size_t bytes_transferred);

    int _port;
    DataCallback _onDataReceived;
    
    asio::io_context _ioContext;
    std::unique_ptr<asio::ip::udp::socket> _socket;
    asio::ip::udp::endpoint _remoteEndpoint;
    std::vector<char> _recvBuffer;
    
    std::thread _serviceThread;
    std::atomic<bool> _running{false};
};

} // namespace Network
