#include "user_info.hpp"

#include <iostream>
#include <comdef.h>
#include <Lmcons.h>
#include <Wbemidl.h>
#include <comutil.h>
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wbemuuid.lib")

namespace user_info {
    static std::string GetWMIProperty(const BSTR wqlClass, const BSTR property) {
        HRESULT hres;
        std::string result;

        // Step 1: Initialize COM
        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) return "CoInitializeEx failed";

        // Step 2: Initialize COM security
        hres = CoInitializeSecurity(
            NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, EOAC_NONE, NULL
        );
        if (FAILED(hres)) return "CoInitializeSecurity failed";

        // Step 3: Connect to WMI
        IWbemLocator* pLocator = nullptr;
        hres = CoCreateInstance(
            CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&pLocator
        );
        if (FAILED(hres)) return "CoCreateInstance failed";

        IWbemServices* pServices = nullptr;
        hres = pLocator->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0,
            NULL, 0, 0, &pServices
        );
        if (FAILED(hres)) return "ConnectServer failed";

        // Set proxy
        hres = CoSetProxyBlanket(
            pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
            NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, EOAC_NONE
        );
        if (FAILED(hres)) return "CoSetProxyBlanket failed";

        // Step 4: Query WMI class
        IEnumWbemClassObject* pEnumerator = nullptr;
        hres = pServices->ExecQuery(
            _bstr_t("WQL"),
            _bstr_t(std::wstring(L"SELECT * FROM ").append(wqlClass).c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL, &pEnumerator
        );
        if (FAILED(hres)) return "ExecQuery failed";

        IWbemClassObject* pClassObject = nullptr;
        ULONG uReturn = 0;

        if (pEnumerator) {
            hres = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
            if (uReturn) {
                VARIANT vtProp;
                hres = pClassObject->Get(property, 0, &vtProp, 0, 0);
                if (SUCCEEDED(hres) && vtProp.vt == VT_BSTR) {
                    result = _bstr_t(vtProp.bstrVal);
                }
                else {
                    result = "<invalid>";
                }
                VariantClear(&vtProp);
                pClassObject->Release();
            }
            pEnumerator->Release();
        }

        pServices->Release();
        pLocator->Release();
        CoUninitialize();

        return result;
    }

    std::string getUUID() {
        return GetWMIProperty(_bstr_t(L"Win32_ComputerSystemProduct"), _bstr_t(L"UUID"));
    }

    std::string getUsername() {
        char username[UNLEN + 1];
        DWORD username_len = UNLEN + 1;
        if (GetUserNameA(username, &username_len)) {
            return std::string(username);
        }
        else {
            return "<unknown>";
        }
    }
}