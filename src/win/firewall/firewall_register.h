#pragma once

#ifdef WIN32
#include <Windows.h>

VOID AddFirewallRuleForThisService();
VOID RemoveFirewallRuleForThisService();
#endif