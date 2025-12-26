#include "UdpSender.hpp"

namespace Network {

UdpSender::UdpSender(const std::string& address, int port) 
    : _targetAddress(address), _targetPort(port) {}

UdpSender::~UdpSender() {
    stop();
}

void UdpSender::start() {
    if (_running) return;

    try {
        _socket = std::make_unique<asio::ip::udp::socket>(_ioContext);
        _socket->open(asio::ip::udp::v4());
        
        _remoteEndpoint = asio::ip::udp::endpoint(asio::ip::make_address(_targetAddress), _targetPort);
        
        _running = true;

        _serviceThread = std::thread([this]() {
            try {
                asio::io_context::work work(_ioContext); // Keep io_context alive
                _ioContext.run();
            } catch (const std::exception& e) {
                std::cerr << "UDP Sender Error: " << e.what() << std::endl;
            }
        });
    } catch (const std::exception& e) {
        std::cerr << "Failed to start UDP Sender: " << e.what() << std::endl;
        _running = false;
    }
}

void UdpSender::stop() {
    if (!_running) return;
    _running = false;

    _ioContext.stop();
    if (_serviceThread.joinable()) {
        _serviceThread.join();
    }
    
    if (_socket && _socket->is_open()) {
        _socket->close();
    }
    
    _ioContext.restart();
    _sendQueue.clear();
    _isSending = false;
}

void UdpSender::send(const std::string& data) {
    if (!_running) return;
    asio::post(_ioContext, [this, data]() {
        doSend(data);
    });
}

void UdpSender::doSend(const std::string& data) {
    _sendQueue.push_back(data);
    if (!_isSending) {
        checkSendQueue();
    }
}

void UdpSender::checkSendQueue() {
    if (_sendQueue.empty()) {
        _isSending = false;
        return;
    }

    _isSending = true;
    const std::string& msg = _sendQueue.front();

    _socket->async_send_to(asio::buffer(msg), _remoteEndpoint,
        [this](const std::error_code& error, std::size_t /*bytes_transferred*/) {
            if (!_running) return;
            
            if (error) {
                std::cerr << "UDP Send Error: " << error.message() << std::endl;
            }

            _sendQueue.pop_front();
            checkSendQueue();
        });
}

} // namespace Network
