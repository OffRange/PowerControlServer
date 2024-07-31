#include "service_report.h"

#ifdef WIN32
#include <strsafe.h>

#include "service_definitions.h"
#include "service_globals.h"
#include "service_msg.h"

VOID ReportServiceEvent(LPCTSTR szFunction) {
    HANDLE  eventSource;
    LPCTSTR strings[2];
    TCHAR   buffer[80];

    eventSource = RegisterEventSource(NULL, SERVICE_NAME);
    if (eventSource != NULL) {
        StringCchPrintf(buffer, ARRAYSIZE(buffer), TEXT("%s failed with %d"),
                        szFunction, GetLastError());

        strings[0] = SERVICE_NAME;
        strings[1] = buffer;

        ReportEvent(/*hEventLog     */ eventSource,
                    /*wType         */ EVENTLOG_ERROR_TYPE,
                    /*wCategory     */ 0,
                    /*dwEventID     */ SVC_ERROR,
                    /*lpUserSid     */ NULL,
                    /*wNumStrings   */ ARRAYSIZE(strings),
                    /*dwDataSize    */ 0,
                    /*lpStrings     */ strings,
                    /*lpRawData     */ NULL);

        DeregisterEventSource(eventSource);
    }
}

VOID ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode,
                         DWORD dwWaitHint) {
    static DWORD dwCheckPoint = 1;

    gSvcStatus.dwCurrentState  = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint      = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING) {
        gSvcStatus.dwControlsAccepted = 0;
    } else {
        gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    if (dwCurrentState == SERVICE_RUNNING ||
        dwCurrentState == SERVICE_STOPPED) {
        gSvcStatus.dwCheckPoint = 0;
    } else {
        gSvcStatus.dwCheckPoint = dwCheckPoint++;
    }

    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}
#endif