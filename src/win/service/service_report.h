#pragma once

#ifdef WIN32
#include <Windows.h>

VOID ReportServiceEvent(LPCTSTR szFunction);

VOID ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode,
                         DWORD dwWaitHint);
#endif