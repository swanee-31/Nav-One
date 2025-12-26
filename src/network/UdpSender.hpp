#pragma once

#include "IService.hpp"
#include <asio.hpp>
#include <thread>
#include <atomic>
#include <string>
#include <deque>
#include <iostream>

namespace Network {

class UdpSender : public IService {
public:
    UdpSender(const std::string& address, int port);
    ~UdpSender();

    void start() override;
    void stop() override;
    bool isRunning() const override { return _running; }
    void send(const std::string& data) override;

private:
    void doSend(const std::string& data);
    void checkSendQueue();

    std::string _targetAddress;
    int _targetPort;
    
    asio::io_context _ioContext;
    std::unique_ptr<asio::ip::udp::socket> _socket;
    asio::ip::udp::endpoint _remoteEndpoint;
    
    std::deque<std::string> _sendQueue;
    bool _isSending = false;

    std::thread _serviceThread;
    std::atomic<bool> _running{false};
};

} // namespace Network
