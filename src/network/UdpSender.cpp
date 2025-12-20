#include "UdpSender.hpp"

namespace Network {

UdpSender::UdpSender(const std::string& address, int port) 
    : targetAddress(address), targetPort(port) {}

UdpSender::~UdpSender() {
    stop();
}

void UdpSender::start() {
    if (running) return;

    try {
        socket = std::make_unique<asio::ip::udp::socket>(ioContext);
        socket->open(asio::ip::udp::v4());
        
        remoteEndpoint = asio::ip::udp::endpoint(asio::ip::make_address(targetAddress), targetPort);
        
        running = true;

        serviceThread = std::thread([this]() {
            try {
                asio::io_context::work work(ioContext); // Keep io_context alive
                ioContext.run();
            } catch (const std::exception& e) {
                std::cerr << "UDP Sender Error: " << e.what() << std::endl;
            }
        });
    } catch (const std::exception& e) {
        std::cerr << "Failed to start UDP Sender: " << e.what() << std::endl;
        running = false;
    }
}

void UdpSender::stop() {
    if (!running) return;
    running = false;

    ioContext.stop();
    if (serviceThread.joinable()) {
        serviceThread.join();
    }
    
    if (socket && socket->is_open()) {
        socket->close();
    }
    
    ioContext.restart();
    sendQueue.clear();
    isSending = false;
}

void UdpSender::send(const std::string& data) {
    if (!running) return;
    asio::post(ioContext, [this, data]() {
        doSend(data);
    });
}

void UdpSender::doSend(const std::string& data) {
    sendQueue.push_back(data);
    if (!isSending) {
        checkSendQueue();
    }
}

void UdpSender::checkSendQueue() {
    if (sendQueue.empty()) {
        isSending = false;
        return;
    }

    isSending = true;
    const std::string& msg = sendQueue.front();

    socket->async_send_to(asio::buffer(msg), remoteEndpoint,
        [this](const std::error_code& error, std::size_t /*bytes_transferred*/) {
            if (!running) return;
            
            if (error) {
                std::cerr << "UDP Send Error: " << error.message() << std::endl;
            }

            sendQueue.pop_front();
            checkSendQueue();
        });
}

} // namespace Network
