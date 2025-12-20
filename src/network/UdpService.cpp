#include "UdpService.hpp"

namespace Network {

UdpService::UdpService(int p, DataCallback callback) 
    : port(p), onDataReceived(callback), recvBuffer(4096) { // 4KB buffer
}

UdpService::~UdpService() {
    stop();
}

void UdpService::start() {
    if (running) return;
    running = true;

    try {
        socket = std::make_unique<asio::ip::udp::socket>(ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port));
        startReceive();

        serviceThread = std::thread([this]() {
            try {
                ioContext.run();
            } catch (const std::exception& e) {
                std::cerr << "UDP Service Error: " << e.what() << std::endl;
            }
        });
    } catch (const std::exception& e) {
        std::cerr << "Failed to start UDP Service: " << e.what() << std::endl;
        running = false;
    }
}

void UdpService::stop() {
    if (!running) return;
    running = false;

    if (socket) {
        socket->close();
    }
    ioContext.stop();

    if (serviceThread.joinable()) {
        serviceThread.join();
    }
}

void UdpService::startReceive() {
    if (!running || !socket) return;

    socket->async_receive_from(
        asio::buffer(recvBuffer), remoteEndpoint,
        [this](const std::error_code& error, std::size_t bytes_transferred) {
            handleReceive(error, bytes_transferred);
        }
    );
}

void UdpService::handleReceive(const std::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        if (bytes_transferred > 0 && onDataReceived) {
            // Create a copy of the data to pass to callback
            std::vector<char> data(recvBuffer.begin(), recvBuffer.begin() + bytes_transferred);
            std::string source = remoteEndpoint.address().to_string() + ":" + std::to_string(remoteEndpoint.port());
            onDataReceived(data, source);
        }
        startReceive(); // Continue listening
    } else {
        if (error != asio::error::operation_aborted) {
            std::cerr << "UDP Receive Error: " << error.message() << std::endl;
            // Try to restart receive if it wasn't a stop command
            if (running) startReceive(); 
        }
    }
}

} // namespace Network
