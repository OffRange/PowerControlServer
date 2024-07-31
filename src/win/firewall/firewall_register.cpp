#include "firewall_register.h"

#ifdef WIN32
#include <netfw.h>
#include <stdio.h>

#include "definitions.h"

VOID AddFirewallRuleForThisService() {
    HRESULT comInit;
    HRESULT result;

    INetFwPolicy2* policy = NULL;
    INetFwRules*   rules  = NULL;
    INetFwRule*    rule   = NULL;

    long currentProfilesBitMask = 0;

    TCHAR executablePath[MAX_PATH];

    if (!GetModuleFileName(NULL, executablePath, MAX_PATH)) {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }

    BSTR ruleName        = SysAllocString(TEXT(APP_NAME));
    BSTR ruleDescription = SysAllocString(
        TEXT("Allow incoming network trafic to Power Control Server"));
    BSTR ruleService = SysAllocString(L"*");
    BSTR ruleExePath = SysAllocString(executablePath);

    comInit = CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    if (comInit != RPC_E_CHANGED_MODE && FAILED(comInit)) {
        printf("CoInitializeEx failed (%x)\n", comInit);
        goto cleanup;
    }

    result =
        CoCreateInstance(__uuidof(NetFwPolicy2), NULL, CLSCTX_INPROC_SERVER,
                         __uuidof(INetFwPolicy2), (void**)&policy);
    if (FAILED(result)) {
        printf("CoCreateInstance for policy failed (%x)\n", result);
        goto cleanup;
    }

    result = policy->get_Rules(&rules);
    if (FAILED(result)) {
        printf("Getting rules failed (%x)\n", result);
        goto cleanup;
    }

    // Retrieve Current Profiles bitmask
    result = policy->get_CurrentProfileTypes(&currentProfilesBitMask);
    if (FAILED(result)) {
        printf("Getting current profile types failed (%x)\n", result);
        goto cleanup;
    }

    // When possible we avoid adding firewall rules to the Public profile.
    // If Public is currently active and it is not the only active profile, we
    // remove it from the bitmask
    if ((currentProfilesBitMask & NET_FW_PROFILE2_PUBLIC) &&
        (currentProfilesBitMask != NET_FW_PROFILE2_PUBLIC)) {
        currentProfilesBitMask ^= NET_FW_PROFILE2_PUBLIC;
    }

    result = CoCreateInstance(__uuidof(NetFwRule), NULL, CLSCTX_INPROC_SERVER,
                              __uuidof(INetFwRule), (void**)&rule);
    if (FAILED(result)) {
        printf("CoCreateInstance for Firewall Rule failed (%x)\n", result);
        goto cleanup;
    }

    rule->put_Name(ruleName);
    rule->put_Description(ruleName);
    rule->put_ApplicationName(ruleExePath);
    rule->put_ServiceName(ruleService);
    rule->put_Protocol(NET_FW_IP_PROTOCOL_UDP);
    rule->put_Profiles(currentProfilesBitMask);
    rule->put_Action(NET_FW_ACTION_ALLOW);
    rule->put_Enabled(VARIANT_TRUE);

    result = rules->Add(rule);
    if (FAILED(result)) {
        printf("Could not add Firewall Rule (%x)\n", result);
        goto cleanup;
    }
    printf("Firewall Rule added successfully.\n");

cleanup:
    SysFreeString(ruleName);
    SysFreeString(ruleDescription);
    SysFreeString(ruleExePath);
    SysFreeString(ruleService);

    if (rule != NULL) {
        rule->Release();
    }

    if (rules != NULL) {
        rules->Release();
    }

    if (policy != NULL) {
        policy->Release();
    }

    if (SUCCEEDED(comInit)) {
        CoUninitialize();
    }
}

VOID RemoveFirewallRuleForThisService() {
    HRESULT comInit;
    HRESULT result;

    INetFwPolicy2* policy = NULL;
    INetFwRules*   rules  = NULL;

    BSTR ruleName = SysAllocString(TEXT(APP_NAME));

    comInit = CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    if (comInit != RPC_E_CHANGED_MODE && FAILED(comInit)) {
        printf("CoInitializeEx failed (%x)\n", comInit);
        goto cleanup;
    }

    result =
        CoCreateInstance(__uuidof(NetFwPolicy2), NULL, CLSCTX_INPROC_SERVER,
                         __uuidof(INetFwPolicy2), (void**)&policy);
    if (FAILED(result)) {
        printf("CoCreateInstance for policy failed (%x)\n", result);
        goto cleanup;
    }

    result = policy->get_Rules(&rules);
    if (FAILED(result)) {
        printf("Getting rules failed (%x)\n", result);
        goto cleanup;
    }

    result = rules->Remove(ruleName);
    if (FAILED(result)) {
        printf("Could not remove Firewall Rule (%x)\n", result);
        goto cleanup;
    }
    printf("Firewall Rule removed successfully.\n");

cleanup:
    SysFreeString(ruleName);

    if (rules != NULL) {
        rules->Release();
    }

    if (policy != NULL) {
        policy->Release();
    }

    if (SUCCEEDED(comInit)) {
        CoUninitialize();
    }
}
#endif