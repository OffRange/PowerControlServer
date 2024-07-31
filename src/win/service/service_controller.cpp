#include <algorithm>
#include <stdio.h>

#include "service_controler.h"
#include "service_definitions.h"

VOID __stdcall StartService() {
    SERVICE_STATUS_PROCESS statusProcess;
    DWORD                  startTime = GetTickCount64();
    DWORD                  oldCheckPoint;
    DWORD                  bytesNeeded;
    DWORD                  waitTime;

    SC_HANDLE serviceControlManager;
    SC_HANDLE service;

    serviceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (serviceControlManager == NULL) {
        printf(
            "Failed to connect to Service Control Manager. Ensure this is run "
            "as a service. (%d)\n",
            GetLastError());
        return;
    }

    service = OpenService(
        serviceControlManager, SERVICE_NAME,
        SERVICE_START | SERVICE_QUERY_STATUS);  // For now, we do not enumerate
                                                // depending services

    if (service == NULL) {
        printf("Opening Service failed (%d)\n", GetLastError());
        CloseServiceHandle(serviceControlManager);
        return;
    }

    if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                              (LPBYTE)&statusProcess,
                              sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        printf("Could not query service status (%d)\n", GetLastError());
        goto stopAndCleanup;
    }

    if (statusProcess.dwCurrentState != SERVICE_STOPPED &&
        statusProcess.dwCurrentState != SERVICE_STOP_PENDING) {
        printf("Service is already running.\n");
        goto stopAndCleanup;
    }

    oldCheckPoint = statusProcess.dwCheckPoint;

    // Wait for the service to stop before attempting to start it.
    while (statusProcess.dwCurrentState == SERVICE_STOP_PENDING) {
        waitTime = std::clamp(statusProcess.dwWaitHint / 10, (DWORD)1000,
                              (DWORD)10000);

        Sleep(waitTime);

        if (!QueryServiceStatusEx(
                service, SC_STATUS_PROCESS_INFO, (LPBYTE)&statusProcess,
                sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
            printf("Could not query service status (%d)\n", GetLastError());
            goto stopAndCleanup;
        }

        if (statusProcess.dwCheckPoint > oldCheckPoint) {
            // Continue to wait and check.

            startTime     = GetTickCount64();
            oldCheckPoint = statusProcess.dwCheckPoint;
        } else {
            if (GetTickCount64() - startTime > statusProcess.dwWaitHint) {
                printf("Timeout waiting for service to stop\n");
                goto stopAndCleanup;
            }
        }
    }

    if (!StartService(service, 0, NULL)) {
        printf("Could not start the service (%d)\n", GetLastError());
        goto stopAndCleanup;
    }

    printf("Service start pending...\n");

    if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                              (LPBYTE)&statusProcess,
                              sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        printf("Could not query service status (%d)\n", GetLastError());
        goto stopAndCleanup;
    }

    startTime     = GetTickCount64();
    oldCheckPoint = statusProcess.dwCheckPoint;

    while (statusProcess.dwCurrentState != SERVICE_RUNNING) {
        // Do not wait longer than the wait hint. A good interval is
        // one-tenth of the wait hint but not less than 1 second
        // and not more than 10 seconds.

        waitTime = std::clamp(statusProcess.dwWaitHint / 10, (DWORD)1000,
                              (DWORD)10000);
        Sleep(waitTime);

        if (!QueryServiceStatusEx(
                service, SC_STATUS_PROCESS_INFO, (LPBYTE)&statusProcess,
                sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
            printf("Could not query service status (%d)\n", GetLastError());
            goto stopAndCleanup;
        }

        if (statusProcess.dwCurrentState == SERVICE_RUNNING) {
            break;
        }

        if (GetTickCount64() - waitTime > statusProcess.dwWaitHint) {
            printf("Service start timed out.\n");
            goto stopAndCleanup;
        }
    }

    printf("Service started successfully.\n");

stopAndCleanup:
    CloseServiceHandle(serviceControlManager);
    CloseServiceHandle(service);
}

VOID __stdcall StopService() {
    SERVICE_STATUS_PROCESS statusProcess;
    DWORD                  startTime = GetTickCount64();
    DWORD                  bytesNeeded;
    DWORD                  timeout = 3000;
    DWORD                  waitTime;

    SC_HANDLE serviceControlManager;
    SC_HANDLE service;

    serviceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (serviceControlManager == NULL) {
        printf("Opening Service Control Manager failed (%d)\n", GetLastError());
        return;
    }

    service = OpenService(
        serviceControlManager, SERVICE_NAME,
        SERVICE_STOP | SERVICE_QUERY_STATUS);  // For now, we do not enumerate
                                               // depending services

    if (service == NULL) {
        printf("Opening Service failed (%d)\n", GetLastError());
        CloseServiceHandle(serviceControlManager);
        return;
    }

    if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                              (LPBYTE)&statusProcess,
                              sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        printf("Could not query service status (%d)\n", GetLastError());
        goto stopAndCleanup;
    }

    if (statusProcess.dwCurrentState == SERVICE_STOPPED) {
        printf("Service already stopped.\n");
        goto stopAndCleanup;
    }

    // If a stop is pending, wait for it.
    while (statusProcess.dwCurrentState == SERVICE_STOP_PENDING) {
        printf("Service stop pending...\n");

        // Do not wait longer than the wait hint. A good interval is
        // one-tenth of the wait hint but not less than 1 second
        // and not more than 10 seconds.

        waitTime = std::clamp(statusProcess.dwWaitHint / 10, (DWORD)1000,
                              (DWORD)10000);
        Sleep(waitTime);

        if (!QueryServiceStatusEx(
                service, SC_STATUS_PROCESS_INFO, (LPBYTE)&statusProcess,
                sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
            printf("Could not query service status (%d)\n", GetLastError());
            goto stopAndCleanup;
        }

        if (statusProcess.dwCurrentState == SERVICE_STOPPED) {
            printf("Service stopped successfully.\n");
            goto stopAndCleanup;
        }

        if (GetTickCount64() - waitTime > timeout) {
            printf("Service stopp timed out.\n");
            goto stopAndCleanup;
        }
    }

    // Send a stop code to the service.
    if (!ControlService(service, SERVICE_CONTROL_STOP,
                        (LPSERVICE_STATUS)&statusProcess)) {
        printf("ControlService failed (%d)\n", GetLastError());
        goto stopAndCleanup;
    }

    // Wait for the service to stop
    while (statusProcess.dwCurrentState != SERVICE_STOPPED) {
        Sleep(statusProcess.dwWaitHint);

        if (!QueryServiceStatusEx(
                service, SC_STATUS_PROCESS_INFO, (LPBYTE)&statusProcess,
                sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
            printf("Could not query service status (%d)\n", GetLastError());
            goto stopAndCleanup;
        }

        if (statusProcess.dwCurrentState == SERVICE_STOPPED) {
            break;
        }

        if (GetTickCount64() - startTime > timeout) {
            printf("Wait timed out\n");
            goto stopAndCleanup;
        }
    }
    printf("Service stopped successfully.\n");

stopAndCleanup:
    CloseServiceHandle(serviceControlManager);
    CloseServiceHandle(service);
}