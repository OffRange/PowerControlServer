#include "service_config.h"

#ifdef WIN32
#include <PathCch.h>
#include <stdio.h>
#include <strsafe.h>
#include <tchar.h>

#include "service_definitions.h"

#pragma comment(lib, "PathCch.lib")

VOID RegisterEventMessageDLL() {
    HKEY  hkey;
    LONG  result;
    DWORD data;

    TCHAR key[MAX_PATH];
    StringCchPrintf(
        key, MAX_PATH,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s"),
        SERVICE_NAME);

    result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, key, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey,
                            NULL);

    if (result != ERROR_SUCCESS) {
        printf("Could not create Registry Key (%d)\n", GetLastError());
        return;
    }

    TCHAR dllName[MAX_PATH];
    StringCchPrintf(dllName, MAX_PATH, TEXT("%s.dll"), TEXT(SERVICE_DLL_NAME));

    TCHAR unquotedPath[MAX_PATH];
    if (!GetModuleFileName(NULL, unquotedPath, MAX_PATH)) {
        wprintf(L"Cannot get %s path (%d)\n", dllName, GetLastError());
        goto close;
    }

    if (PathCchRemoveFileSpec(unquotedPath, MAX_PATH) == S_FALSE) {
        wprintf(L"Cannot get %s path (%d)\n", dllName, GetLastError());
        goto close;
    }

    if (PathCchAppend(unquotedPath, MAX_PATH, dllName) == S_FALSE) {
        wprintf(L"Cannot get %s path (%d)\n", dllName, GetLastError());
        goto close;
    }

    result = RegSetValueEx(hkey, TEXT("EventMessageFile"), 0, REG_SZ,
                           (const LPBYTE)&unquotedPath,
                           (_tcsclen(unquotedPath) + 1) * sizeof(TCHAR));

    if (result != ERROR_SUCCESS) {
        printf("Could not set EventMessageFile value (%d)\n", GetLastError());
        goto close;
    }

    data =
        EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    result = RegSetValueEx(hkey, TEXT("TypesSupported"), 0, REG_DWORD,
                           (const LPBYTE)&data, sizeof(DWORD));

    if (result != ERROR_SUCCESS) {
        printf("Could not set TypesSupported value (%d)\n", GetLastError());
        goto close;
    }

    printf("Eventlog dll registered successfully.\n");
    goto close;
close:
    RegCloseKey(hkey);
    return;
}

VOID UnregisterEventMessageDll() {
    HKEY hkey;

    TCHAR key[MAX_PATH];
    StringCchPrintf(
        key, MAX_PATH,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s"),
        SERVICE_NAME);

    if (RegDeleteKey(HKEY_LOCAL_MACHINE, key) != ERROR_SUCCESS) {
        printf("Could not create Registry Key (%d)\n", GetLastError());
        return;
    }

    printf("Eventlog dll unregistered successfully.\n");
}

VOID InstallService() {
    SC_HANDLE serviceControlManager;
    SC_HANDLE service;
    TCHAR     unquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, unquotedPath, MAX_PATH)) {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }

    TCHAR execuatableFilePath[MAX_PATH];
    StringCchPrintf(execuatableFilePath, MAX_PATH, TEXT("\"%s\""),
                    unquotedPath);

    serviceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (serviceControlManager == NULL) {
        printf("Opening Service Control Manager failed (%d)\n", GetLastError());
        return;
    }

    service = CreateService(serviceControlManager, SERVICE_NAME, SERVICE_NAME,
                            SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                            SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
                            execuatableFilePath, NULL, NULL, NULL, NULL, NULL);

    if (service == NULL) {
        printf("Creating Service failed (%d)\n", GetLastError());
        CloseServiceHandle(serviceControlManager);
        return;
    }

    printf("Service installed successfully.\n");

    CloseServiceHandle(service);
    CloseServiceHandle(serviceControlManager);

    RegisterEventMessageDLL();
}

VOID UninstallService() {
    SC_HANDLE      serviceControlManager;
    SC_HANDLE      service;
    SERVICE_STATUS status;

    serviceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (serviceControlManager == NULL) {
        printf("Opening Service Control Manager failed (%d)\n", GetLastError());
        return;
    }

    service = OpenService(serviceControlManager, SERVICE_NAME, DELETE);
    if (service == NULL) {
        printf("Opening Service failed (%d)\n", GetLastError());
        CloseServiceHandle(serviceControlManager);
        return;
    }

    if (!DeleteService(service)) {
        printf("Deleting Service failed (%d)\n", GetLastError());
        CloseServiceHandle(serviceControlManager);
        return;
    }

    printf("Service uninstalled successfully.\n");

    CloseServiceHandle(service);
    CloseServiceHandle(serviceControlManager);

    UnregisterEventMessageDll();
}
#endif