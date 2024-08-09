#include "processor.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#include "win/service/service_report.h"
#pragma comment(lib, "ws2_32.lib")

#include <wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")
#else
#include <signal.h>
#include <sys/reboot.h>
#include <unistd.h>
#endif

#include <iomanip>
#include <iostream>

#define HEARTBEAT_BYTE 0x00
#define SHUTDOWN_BYTE 0x01
#define RESTART_BYTE 0x02
#define LOGOUT_BYTE 0x03

void ProcessResponse(const std::vector<unsigned char>& data, SendFunction send,
                     CloseFunction close) {
    std::cout << "Received: 0x";
    for (unsigned char byte : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (static_cast<char>(byte) & 0xff) << " ";
    }
    std::cout << std::dec << std::endl;

    if (data.size() == 1) {
        if ((static_cast<char>(data[0]) & 0xff) == HEARTBEAT_BYTE) {
            RespondToHeartbeat(send);
        }
    } else if (data.size() == 2) {
        switch (static_cast<char>(data[0]) & 0xff) {
            case SHUTDOWN_BYTE:
                if ((static_cast<char>(data[1]) & 0xff) == 0xa5) {
                    close();
                    if (!Shutdown()) {
                        std::cerr << "Shutdown failed!" << std::endl;
                    }
                }
                break;
            case RESTART_BYTE:
                if ((static_cast<char>(data[1]) & 0xff) == 0xa5) {
                    close();
                    if (!Restart()) {
                        std::cerr << "Restart failed!" << std::endl;
                    }
                }
                break;
            case LOGOUT_BYTE:
                if ((static_cast<char>(data[1]) & 0xff) == 0xa5) {
                    if (!Logout()) {
                        std::cerr << "Logout failed!" << std::endl;
                    }
                }
                break;
            default:
                break;
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

bool Restart() {
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

    return ExitWindowsEx(EWX_REBOOT | EWX_FORCE,
                         SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                             SHTDN_REASON_MINOR_UPGRADE |
                             SHTDN_REASON_FLAG_PLANNED);
#else
    sync();

    return reboot(RB_AUTOBOOT) != 0;
#endif  // _WIN32
}

bool Logout() {
#ifdef _WIN32
    PWTS_SESSION_INFO pSessionInfo = NULL;
    DWORD             dwCount      = 0;

    if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo,
                             &dwCount)) {
        for (DWORD i = 0; i < dwCount; i++) {
            WTS_SESSION_INFO si = pSessionInfo[i];
            if (si.State == WTSActive) {
                return WTSLogoffSession(WTS_CURRENT_SERVER_HANDLE, si.SessionId,
                                        FALSE);
            }
        }
        WTSFreeMemory(pSessionInfo);
    }
    return false;
#else
    // TODO
#endif  // _WIN32
}

void RespondToHeartbeat(SendFunction send) { send({0x00}); }