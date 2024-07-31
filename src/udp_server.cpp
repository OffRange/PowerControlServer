#include "udp_server.h"

#include <memory.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <iostream>
#include <memory>
#include <stdexcept>

UdpServer::UdpServer(const Port port)
    : port_(port), socket_(INVALID_UDP_SOCKET) {}

UdpServer::~UdpServer() { Close(); }

void UdpServer::ReceiveContinuous(ResponseCallback callback) {
    unsigned char buffer[BUFFER_SIZE];
    started_ = true;

    sockaddr_in clientAddr;
    socklen_t   client_addr_len = sizeof(clientAddr);

    while (started_) {
        std::cout << "Waiting for data..." << std::endl;
        memset(buffer, '\0', BUFFER_SIZE);

        size_t bytesRead = recvfrom(
            socket_, reinterpret_cast<char*>(&buffer), BUFFER_SIZE, 0,
            reinterpret_cast<sockaddr*>(&clientAddr), &client_addr_len);
        if (bytesRead == UDP_SOCKET_ERROR) {
            // TODO send a msg
            std::cerr << "recvfrom() failed with error code: <code>"
                      << std::endl;

            continue;
        }

        auto vector = std::vector<unsigned char>(buffer, buffer + bytesRead);
        callback(
            vector,
            [&](const std::vector<unsigned char>& data) {
                size_t bytesSent = sendto(
                    socket_, reinterpret_cast<const char*>(data.data()),
                    data.size(), 0, reinterpret_cast<sockaddr*>(&clientAddr),
                    client_addr_len);

                if (bytesSent == UDP_SOCKET_ERROR) {
                    std::cerr << "sendto() failed with error code: <code>"
                              << std::endl;
                }
            },
            [&]() { Close(); });
    }
}

void UdpServer::Close() {
    started_ = false;
    if (socket_ == INVALID_UDP_SOCKET) {
        return;
    }

#ifdef _WIN32
    closesocket(socket_);
    WSACleanup();
#else
    shutdown(socket_, SHUT_RDWR);
    close(socket_);
#endif
}

void UdpServer::Start() {
#ifdef _WIN32
    WSADATA wsaData;

    int error_code;
    if ((error_code = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
        std::cerr << "WSAStartup failed with code: <code>" << std::endl;
        return;
    }
#endif
    // Currently only IPv4 is supported

    if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_UDP_SOCKET) {
        Close();

        throw std::runtime_error(
            "Error while initiating a socket; error = <code>");
    }

    // Create the addresses
    sockaddr_in serverAddr;
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_port        = htons(port_);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_, reinterpret_cast<sockaddr*>(&serverAddr),
             sizeof(serverAddr)) < 0) {
        Close();
        throw std::runtime_error(
            "Error while binding a socket; error = <code>");
    }

    // getsockname
    std::cout << "UDP Server listening on port " << port_ << std::endl;
}