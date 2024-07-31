#include "service_globals.h"

#ifdef WIN32
SERVICE_STATUS        gSvcStatus;
SERVICE_STATUS_HANDLE gSvcStatusHandle;
HANDLE                gSvcStopEvent = INVALID_HANDLE_VALUE;
#endif