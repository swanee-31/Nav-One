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
    bool isRunning() const override { return running; }
    void send(const std::string& data) override;

private:
    void doSend(const std::string& data);
    void checkSendQueue();

    std::string targetAddress;
    int targetPort;
    
    asio::io_context ioContext;
    std::unique_ptr<asio::ip::udp::socket> socket;
    asio::ip::udp::endpoint remoteEndpoint;
    
    std::deque<std::string> sendQueue;
    bool isSending = false;

    std::thread serviceThread;
    std::atomic<bool> running{false};
};

} // namespace Network
