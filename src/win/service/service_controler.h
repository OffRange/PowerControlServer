#pragma once

#ifdef WIN32
#include <Windows.h>

VOID __stdcall StartService();
VOID __stdcall StopService();
#endif