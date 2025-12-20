#pragma once

#include "IService.hpp"
#include <asio.hpp>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <deque>

namespace Network {

class SerialService : public IService {
public:
    using DataCallback = std::function<void(const std::vector<char>&, const std::string&)>;

    SerialService(const std::string& portName, unsigned int baudRate, DataCallback callback);
    ~SerialService();

    void start() override;
    void stop() override;
    bool isRunning() const override { return running; }
    void send(const std::string& data) override;

private:
    void startReceive();
    void handleReceive(const std::error_code& error, std::size_t bytes_transferred);
    void doWrite(const std::string& data);
    void checkWriteQueue();

    std::string portName;
    unsigned int baudRate;
    DataCallback onDataReceived;
    
    asio::io_context ioContext;
    std::unique_ptr<asio::serial_port> serialPort;
    std::vector<char> recvBuffer;
    
    std::deque<std::string> writeQueue;
    bool isWriting = false;

    std::thread serviceThread;
    std::atomic<bool> running{false};
};

} // namespace Network
