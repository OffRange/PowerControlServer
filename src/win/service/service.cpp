#include "service.h"

#ifdef WIN32
#include <tchar.h>  // For _tmain

#include "processor.h"
#include "service_config.h"
#include "service_controler.h"
#include "service_definitions.h"
#include "service_globals.h"
#include "service_report.h"
#include "udp_server.h"
#include "win/firewall/firewall_register.h"

static UdpServer server(DEFAULT_PORT);
int __cdecl _tmain(int argc, TCHAR* argv[]) {
    if (lstrcmpi(argv[1], TEXT("install")) == 0) {
        InstallService();
        AddFirewallRuleForThisService();
        StartService();
        return 0;
    }

    if (lstrcmpi(argv[1], TEXT("uninstall")) == 0) {
        StopService();
        UninstallService();
        RemoveFirewallRuleForThisService();
        return 0;
    }

    SERVICE_TABLE_ENTRY dispatchTable[] = {
        {const_cast<LPWSTR>(SERVICE_NAME),
         (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {NULL, NULL}};

    if (!StartServiceCtrlDispatcher(dispatchTable)) {
        ReportServiceEvent(TEXT("StartServiceCtrlDispatcher"));
        return -1;
    }
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPCTSTR* lpszArgv) {
    gSvcStatusHandle =
        RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (!gSvcStatusHandle) {
        ReportServiceEvent(TEXT("RegisterServiceCtrlHandler"));
        return;
    }

    gSvcStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    ServiceInit(dwArgc, lpszArgv);
}

// ------
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam) {
    server.Start();
    server.ReceiveContinuous(ProcessResponse);
    server.Close();

    return ERROR_SUCCESS;
}

VOID ServiceInit(DWORD dwArgc, LPCTSTR* lpszArgv) {
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    gSvcStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (gSvcStopEvent == NULL) {
        ReportServiceStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

    // TODO work
    HANDLE thread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);
    // WaitForSingleObject(thread, INFINITE);

    while (1) {
        WaitForSingleObject(gSvcStopEvent, INFINITE);
        CloseHandle(gSvcStopEvent);
        ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }
}

// ----

VOID WINAPI ServiceCtrlHandler(DWORD dwCtrl) {
    switch (dwCtrl) {
        case SERVICE_CONTROL_STOP:
            ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
            server.Close();

            // This will signal the worker thread to start shutting down
            SetEvent(gSvcStopEvent);
            ReportServiceStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
            return;
        default:
            break;
    }
}
#endif