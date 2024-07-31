#pragma once

#ifdef WIN32
#include <Windows.h>

extern SERVICE_STATUS        gSvcStatus;
extern SERVICE_STATUS_HANDLE gSvcStatusHandle;
extern HANDLE                gSvcStopEvent;
#endif