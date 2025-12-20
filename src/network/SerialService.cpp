#include "SerialService.hpp"
#include <iostream>

namespace Network {

SerialService::SerialService(const std::string& port, unsigned int baud, DataCallback callback) 
    : portName(port), baudRate(baud), onDataReceived(callback), recvBuffer(1024) {
}

SerialService::~SerialService() {
    stop();
}

void SerialService::start() {
    if (running) return;
    
    try {
        serialPort = std::make_unique<asio::serial_port>(ioContext);
        serialPort->open(portName);
        
        serialPort->set_option(asio::serial_port_base::baud_rate(baudRate));
        serialPort->set_option(asio::serial_port_base::character_size(8));
        serialPort->set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));
        serialPort->set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
        serialPort->set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));

        running = true;
        startReceive();

        serviceThread = std::thread([this]() {
            try {
                ioContext.run();
            } catch (const std::exception& e) {
                std::cerr << "Serial Service Error: " << e.what() << std::endl;
            }
        });
    } catch (const std::exception& e) {
        std::cerr << "Failed to start Serial Service on " << portName << ": " << e.what() << std::endl;
        running = false;
    }
}

void SerialService::stop() {
    if (!running) return;
    running = false;

    if (serialPort && serialPort->is_open()) {
        serialPort->cancel();
        serialPort->close();
    }
    ioContext.stop();

    if (serviceThread.joinable()) {
        serviceThread.join();
    }
    
    // Reset ioContext for potential restart
    ioContext.restart();
}

void SerialService::startReceive() {
    if (!running || !serialPort) return;

    serialPort->async_read_some(
        asio::buffer(recvBuffer),
        [this](const std::error_code& error, std::size_t bytes_transferred) {
            handleReceive(error, bytes_transferred);
        }
    );
}

void SerialService::handleReceive(const std::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        if (bytes_transferred > 0 && onDataReceived) {
            std::vector<char> data(recvBuffer.begin(), recvBuffer.begin() + bytes_transferred);
            onDataReceived(data, portName);
        }
        startReceive();
    } else {
        if (error != asio::error::operation_aborted) {
            std::cerr << "Serial Receive Error: " << error.message() << std::endl;
            // Don't restart immediately on error to avoid tight loops on disconnected devices
            running = false; 
        }
    }
}

} // namespace Network
