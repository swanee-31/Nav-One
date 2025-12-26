#include "UdpService.hpp"

namespace Network {

UdpService::UdpService(int p, DataCallback callback) 
    : _port(p), _onDataReceived(callback), _recvBuffer(4096) { // 4KB buffer
}

UdpService::~UdpService() {
    stop();
}

void UdpService::start() {
    if (_running) return;
    _running = true;

    try {
        _socket = std::make_unique<asio::ip::udp::socket>(_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), _port));
        startReceive();

        _serviceThread = std::thread([this]() {
            try {
                _ioContext.run();
            } catch (const std::exception& e) {
                std::cerr << "UDP Service Error: " << e.what() << std::endl;
            }
        });
    } catch (const std::exception& e) {
        std::cerr << "Failed to start UDP Service: " << e.what() << std::endl;
        _running = false;
    }
}

void UdpService::stop() {
    if (!_running) return;
    _running = false;

    if (_socket) {
        _socket->close();
    }
    _ioContext.stop();

    if (_serviceThread.joinable()) {
        _serviceThread.join();
    }
}

void UdpService::startReceive() {
    if (!_running || !_socket) return;

    _socket->async_receive_from(
        asio::buffer(_recvBuffer), _remoteEndpoint,
        [this](const std::error_code& error, std::size_t bytes_transferred) {
            handleReceive(error, bytes_transferred);
        }
    );
}

void UdpService::handleReceive(const std::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        if (bytes_transferred > 0 && _onDataReceived) {
            // Create a copy of the data to pass to callback
            std::vector<char> data(_recvBuffer.begin(), _recvBuffer.begin() + bytes_transferred);
            std::string source = _remoteEndpoint.address().to_string() + ":" + std::to_string(_remoteEndpoint.port());
            _onDataReceived(data, source);
        }
        startReceive(); // Continue listening
    } else {
        if (error != asio::error::operation_aborted) {
            std::cerr << "UDP Receive Error: " << error.message() << std::endl;
            // Try to restart receive if it wasn't a stop command
            if (_running) startReceive(); 
        }
    }
}

} // namespace Network
