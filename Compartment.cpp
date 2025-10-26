#include <winsock2.h>
#include <ws2ipdef.h>
#include <windows.h>
#include <netioapi.h>
#include <iphlpapi.h>
#include <netlistmgr.h>   // Network List Manager interfaces
#include <objbase.h>     // CoInitializeEx
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

typedef struct _COMPARTMENT_ENRTY
{
    GUID    Guid;              
    DWORD   CompartmentId;
    WORD    DescriptionSize;
    WCHAR   Description[257];
    DWORD   Flags;
    // Structure size: 0x21C (540 bytes)
} COMPARTMENT_ENRTY, * PCOMPARTMENT_ENRTY;

typedef DWORD(WINAPI* PFN_CreateCompartment)(PCOMPARTMENT_ENRTY pEntry);
typedef DWORD(WINAPI* PFN_DeleteCompartment)(DWORD CompartmentId, DWORD Reserved);
typedef NTSTATUS(WINAPI* PFN_SetNetworkInformation)(const NET_IF_NETWORK_GUID* NetworkGuid, NET_IF_COMPARTMENT_ID CompartmentId, const WCHAR* NetworkName);

int EnumerateNetworks()
{
    size_t networksCount = 0;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        std::wcerr << L"EnumerateNetworks:: CoInitializeEx failed: 0x" << std::hex << hr << L"\n";
        return 0;
    }

    INetworkListManager* pNLM = nullptr;
    hr = CoCreateInstance(CLSID_NetworkListManager,
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&pNLM));
    if (FAILED(hr))
    {
        std::wcerr << L"EnumerateNetworks:: CoCreateInstance(CLSID_NetworkListManager) failed: 0x" << std::hex << hr << L"\n";
        CoUninitialize();
        return 0;
    }

    IEnumNetworks* pEnum = nullptr;
    hr = pNLM->GetNetworks(NLM_ENUM_NETWORK_ALL, &pEnum);
    if (FAILED(hr))
    {
        std::wcerr << L"EnumerateNetworks:: GetNetworks failed: 0x" << std::hex << hr << L"\n";
        pNLM->Release();
        CoUninitialize();
        return 0;
    }

    INetwork* pNetwork = nullptr;
    ULONG fetched = 0;
    std::wcout << L"EnumerateNetworks::Network GUIDs:\n";

    while (SUCCEEDED(hr = pEnum->Next(1, &pNetwork, &fetched)) && fetched == 1)
    {
        GUID netId;
        hr = pNetwork->GetNetworkId(&netId);
        if (SUCCEEDED(hr))
        {
            WCHAR guidStr[40]; // GUID string is 38 chars + braces + null
            if (0 != StringFromGUID2(netId, guidStr, ARRAYSIZE(guidStr)))
            {
                std::wcout << L"  " << guidStr << L"\n";

                ++networksCount;
            }
            else
            {
                std::wcout << L"  (failed to convert GUID)\n";
            }
        }
        else
        {
            std::wcout << L"  (GetNetworkId failed: 0x" << std::hex << hr << L")\n";
        }

        pNetwork->Release();
        pNetwork = nullptr;
    }

    // cleanup
    pEnum->Release();
    pNLM->Release();
    CoUninitialize();

    return networksCount;
}

void InitCompartmentEntry(PCOMPARTMENT_ENRTY pEntry, const GUID* pGuid, const WCHAR* desc)
{
    memset(pEntry, 0, sizeof(COMPARTMENT_ENRTY));

    CoCreateGuid(&pEntry->Guid);  // Generate new GUID

    WCHAR CompartmentGuid[40] = {0};
    StringFromGUID2(pEntry->Guid, (LPOLESTR)&CompartmentGuid, 39);

    if (desc)
    {
        pEntry->DescriptionSize = (USHORT)(wcslen(desc) * sizeof(WCHAR));
        wcsncpy_s(pEntry->Description, 257, desc, _TRUNCATE);
    }

    pEntry->Flags = 0;
}

DWORD CreateCompartment(const wchar_t* compartmentName)
{
    DWORD CompartmentId = 0;

    std::cout << "=== CreateCompartment '" << compartmentName << L"' === " << std::endl;

    CoInitialize(NULL);

    HMODULE hIpHlpApi = LoadLibraryW(L"iphlpapi.dll");
    if (!hIpHlpApi)
    {
        std::cerr << "CreateCompartment:: Failed to load iphlpapi.dll" << std::endl;
        CoUninitialize();
        return 0;
    }

    PFN_CreateCompartment pCreateCompartment = (PFN_CreateCompartment)GetProcAddress(hIpHlpApi, "CreateCompartment");

    if (!pCreateCompartment)
    {
        std::cerr << "CreateCompartment:: CreateCompartment function not found" << std::endl;
        FreeLibrary(hIpHlpApi);
        CoUninitialize();
        return 0;
    }

    COMPARTMENT_ENRTY entry;
    InitCompartmentEntry(&entry, NULL, compartmentName);

    entry.Flags |= 1;

    DWORD result = pCreateCompartment(&entry);

    std::cout << "\nCreateCompartment:: Result: " << result << " (0x" << std::hex << result << std::dec << ")" << std::endl;

    if (result == ERROR_SUCCESS)
    {
        std::wcout << L"GUID: {" << std::hex << entry.Guid.Data1 << L"-"
            << entry.Guid.Data2 << L"-"
            << entry.Guid.Data3 << L"-"
            << entry.Guid.Data4 << L"}" << std::dec << std::endl;
        std::wcout << L"ID: " << entry.CompartmentId << std::endl;
        std::wcout << L"Name: " << entry.Description << std::endl;

        CompartmentId = entry.CompartmentId;

        system("powershell Get-NetCompartment");
    }
    else
    {
        std::cout << "CreateCompartment:: FAILED: " << result << std::endl;
    }

    FreeLibrary(hIpHlpApi);
    CoUninitialize();

    return CompartmentId;
}

void DeleteCompartment(DWORD CompartmentId)
{
    std::cout << "=== DeleteCompartment '" << CompartmentId << L"' === " << std::endl;

    CoInitialize(NULL);

    HMODULE hIpHlpApi = LoadLibraryW(L"iphlpapi.dll");
    if (!hIpHlpApi)
    {
        std::cerr << "DeleteCompartment:: Failed to load iphlpapi.dll" << std::endl;
        CoUninitialize();
        return;
    }

    PFN_DeleteCompartment pDeleteCompartment = (PFN_DeleteCompartment)GetProcAddress(hIpHlpApi, "DeleteCompartment");

    if (!pDeleteCompartment)
    {
        std::cerr << "DeleteCompartment not found" << std::endl;
        FreeLibrary(hIpHlpApi);
        CoUninitialize();
        return;
    }

    pDeleteCompartment(CompartmentId, 0);

    std::cout << "=== DeleteCompartment:: compartment '" << CompartmentId << L"' was deleted " << std::endl;

    system("powershell Get-NetCompartment");

    FreeLibrary(hIpHlpApi);
    CoUninitialize();
}

void UpdateNetwork(const wchar_t* NetworkGuid, const wchar_t* NetworkName, NET_IF_COMPARTMENT_ID CompartmentId)
{
    std::cout << "=== UpdateNetwork ' " << NetworkGuid << L" ' ===" << std::endl;

    GUID ifaceGuid;
    if (FAILED(CLSIDFromString(NetworkGuid, &ifaceGuid)))
    {
        std::wcerr << L"Invalid GUID string: " << NetworkGuid << std::endl;
        return;
    }

    CoInitialize(NULL);

    HMODULE hIpHlpApi = LoadLibraryW(L"iphlpapi.dll");
    if (!hIpHlpApi)
    {
        std::cerr << "UpdateNetwork:: Failed to load iphlpapi.dll" << std::endl;
        CoUninitialize();
        return;
    }

    PFN_SetNetworkInformation pSetNetworkInformation = (PFN_SetNetworkInformation)GetProcAddress(hIpHlpApi, "SetNetworkInformation");

    if (!pSetNetworkInformation)
    {
        std::cerr << "SetNetworkInformation not found" << std::endl;
        FreeLibrary(hIpHlpApi);
        CoUninitialize();
        return;
    }

    auto ntStatus = pSetNetworkInformation(&ifaceGuid, CompartmentId, NetworkName);

    std::cout << "=== UpdateNetwork:: STATUS= " << std::hex << ntStatus << std::endl;

    FreeLibrary(hIpHlpApi);
    CoUninitialize();
}

int main()
{
    auto networksCount = EnumerateNetworks();

    std::wcout << L"Networks count before compartment: " << networksCount << std::endl;


    auto compId = CreateCompartment(L"MYCOMPARTMENT#1");

    std::wcout << L"Networks count after compartment: " << networksCount << std::endl;


    //UpdateNetwork(L"{4DBCC94C-784A-472A-8B22-D4F7D3ACA1D9}", L"Ethernet1", 2);

    return 0;
}