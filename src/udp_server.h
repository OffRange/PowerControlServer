#pragma once

#define BUFFER_SIZE 2

#include <functional>
#include <vector>

#include "declarations.h"
#include "processor.h"

#ifdef _WIN32
using UdpSocket = unsigned int;
#else
using UdpSocket = int;
#endif

#ifdef _WIN32
#define INVALID_UDP_SOCKET INVALID_SOCKET
#define UDP_SOCKET_ERROR SOCKET_ERROR
#else
#define INVALID_UDP_SOCKET -1
#define UDP_SOCKET_ERROR -1
#endif

class UdpServer {
   private:
    Port port_;
    UdpSocket socket_;
    bool started_ = false;

   public:
    UdpServer(const Port port);
    ~UdpServer();
    void ReceiveContinuous(ResponseCallback callback);
    void Close();
    void Start();
};