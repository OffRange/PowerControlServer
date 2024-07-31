#include <iostream>

#include "definitions.h"
#include "processor.h"
#include "udp_server.h"

int main() {
    UdpServer server(DEFAULT_PORT);
    server.Start();
    server.ReceiveContinuous(ProcessResponse);

    server.Close();

    return 0;
}