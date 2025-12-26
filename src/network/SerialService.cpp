#include "SerialService.hpp"
#include <iostream>

namespace Network {

SerialService::SerialService(const std::string& port, unsigned int baud, DataCallback callback) 
    : _portName(port), _baudRate(baud), _onDataReceived(callback), _recvBuffer(1024) {
}

SerialService::~SerialService() {
    stop();
}

void SerialService::start() {
    if (_running) return;
    
    try {
        _serialPort = std::make_unique<asio::serial_port>(_ioContext);
        _serialPort->open(_portName);
        
        _serialPort->set_option(asio::serial_port_base::baud_rate(_baudRate));
        _serialPort->set_option(asio::serial_port_base::character_size(8));
        _serialPort->set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));
        _serialPort->set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
        _serialPort->set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));

        _running = true;
        startReceive();

        _serviceThread = std::thread([this]() {
            try {
                _ioContext.run();
            } catch (const std::exception& e) {
                std::cerr << "Serial Service Error: " << e.what() << std::endl;
            }
        });
    } catch (const std::exception& e) {
        std::cerr << "Failed to start Serial Service on " << _portName << ": " << e.what() << std::endl;
        _running = false;
    }
}

void SerialService::stop() {
    if (!_running) return;
    _running = false;

    if (_serialPort && _serialPort->is_open()) {
        _serialPort->cancel();
        _serialPort->close();
    }
    _ioContext.stop();

    if (_serviceThread.joinable()) {
        _serviceThread.join();
    }
    
    // Reset ioContext for potential restart
    _ioContext.restart();
    _writeQueue.clear();
    _isWriting = false;
}

void SerialService::startReceive() {
    if (!_running || !_serialPort) return;

    _serialPort->async_read_some(
        asio::buffer(_recvBuffer),
        [this](const std::error_code& error, std::size_t bytes_transferred) {
            handleReceive(error, bytes_transferred);
        }
    );
}

void SerialService::handleReceive(const std::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        if (bytes_transferred > 0 && _onDataReceived) {
            std::vector<char> data(_recvBuffer.begin(), _recvBuffer.begin() + bytes_transferred);
            _onDataReceived(data, _portName);
        }
        startReceive();
    } else {
        if (error != asio::error::operation_aborted) {
            std::cerr << "Serial Receive Error: " << error.message() << std::endl;
            // Don't restart immediately on error to avoid tight loops on disconnected devices
            _running = false; 
        }
    }
}

void SerialService::send(const std::string& data) {
    if (!_running) return;
    asio::post(_ioContext, [this, data]() {
        doWrite(data);
    });
}

void SerialService::doWrite(const std::string& data) {
    _writeQueue.push_back(data);
    if (!_isWriting) {
        checkWriteQueue();
    }
}

void SerialService::checkWriteQueue() {
    if (_writeQueue.empty()) {
        _isWriting = false;
        return;
    }

    _isWriting = true;
    const std::string& msg = _writeQueue.front();

    asio::async_write(*_serialPort, asio::buffer(msg),
        [this](const std::error_code& error, std::size_t /*bytes_transferred*/) {
            if (!_running) return;
            
            if (error) {
                std::cerr << "Serial Write Error: " << error.message() << std::endl;
            }

            _writeQueue.pop_front();
            checkWriteQueue();
        });
}

} // namespace Network
