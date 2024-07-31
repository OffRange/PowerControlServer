#pragma once

#ifdef WIN32
#include <Windows.h>
#pragma comment(lib, "advapi32.lib")

VOID WINAPI ServiceMain(DWORD dwArgc, LPCTSTR* lpszArgv);
VOID        ServiceInit(DWORD dwArgc, LPCTSTR* lpszArgv);

// -------

VOID WINAPI ServiceCtrlHandler(DWORD dwCtrl);
#endif