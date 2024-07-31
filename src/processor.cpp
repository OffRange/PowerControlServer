#include "processor.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/reboot.h>
#include <unistd.h>
#endif

#include <iomanip>
#include <iostream>

void ProcessResponse(const std::vector<std::byte>& data, CloseFunction close) {
    std::cout << "Received: 0x";
    for (std::byte byte : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (static_cast<char>(byte) & 0xff) << " ";
    }
    std::cout << std::dec << std::endl;

    if (data.size() == 1) {
        if ((static_cast<char>(data[0]) & 0xff) == 0xa5) {
            close();
            if (!Shutdown()) {
                std::cerr << "Shutdown failed! " << std::endl;
            }
        }
    }
}

bool Shutdown() {
#ifdef _WIN32
    HANDLE           hToken;
    TOKEN_PRIVILEGES tkp;

    // Get a token for this process.

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return false;

    // Get the LUID for the shutdown privilege.

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount           = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process.

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS) return false;

    return ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,
                         SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                             SHTDN_REASON_MINOR_UPGRADE |
                             SHTDN_REASON_FLAG_PLANNED);
#else
    sync();

    return reboot(RB_POWER_OFF) != 0;
#endif  // _WIN32
}