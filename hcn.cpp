#include <windows.h>
#include <initguid.h>
#include <objbase.h> // For CoCreateGuid
#include <string>
#include "wil\resource.h"

// HCS API header file
#include <computenetwork.h> // For HCN APIs

#pragma comment(lib, "ComputeNetwork.lib")

// {18BA218E-B2FB-463E-B4C4-14C12A49303F}
DEFINE_GUID(networkId, 0x18ba218e, 0xb2fb, 0x463e, 0xb4, 0xc4, 0x14, 0xc1, 0x2a, 0x49, 0x30, 0x3f);

const wchar_t* g_networkGuid = L"66666666-1113-4CD4-A50C-80FBB2972DEB";

GUID StringToGuid(const wchar_t* guidString)
{
    GUID guid;
    HRESULT hr = CLSIDFromString(guidString, &guid);

    if (FAILED(hr))
    {
        // Handle error
        memset(&guid, 0, sizeof(GUID));
    }

    return guid;
}

void CreateHcnNetwork()
{
    using unique_hcn_network = wil::unique_any<
        HCN_NETWORK,
        decltype(&HcnCloseNetwork),
        HcnCloseNetwork>;

    unique_hcn_network hcnnetwork;
    wil::unique_cotaskmem_string errorRecord;
    std::wstring settings = LR"(
    {
         "Name": "MyNatNetwork",
        "Owner": "ME",
        "ID": "66666666-1113-4CD4-A50C-80FBB2972DEB",
    "Type": "NAT",
    "Ipam": {
        "Type": "Static",
        "Subnet": "172.16.0.0/12",
        "GatewayAddress": "172.16.0.1"
    }
    })";

    GUID networkGuid;
    HRESULT result = CoCreateGuid(&networkGuid);

    result = HcnCreateNetwork(
        networkGuid,              // Unique ID
        settings.c_str(),      // Compute system settings document
        &hcnnetwork,
        &errorRecord
    );

    if (FAILED(result))
    {
        printf("HcnCreateNetwork:: error= 0x%X", result);

        wprintf(L"HcnCreateNetwork: ERROR = %ws", errorRecord);

        THROW_HR(result);
    }

    WCHAR networkGuidString[64] = {0};
    int len = StringFromGUID2(networkGuid, networkGuidString, ARRAYSIZE(networkGuidString));

    wprintf(L"HcnCreateNetwork: SUCCESS GUID = %ws", networkGuidString);


    // Close the Handle
    result = HcnCloseNetwork(hcnnetwork.get());
    if (FAILED(result))
    {
        printf("HcnCloseNetwork:: error= 0x%X", result);

        // UnMarshal  the result Json
        THROW_HR(result);
    }
}


void CreateHcnNamespace()
{
    using unique_hcn_namespace = wil::unique_any<
        HCN_NAMESPACE,
        decltype(&HcnCloseNamespace),
        HcnCloseNamespace>;
    /// Creates a simple HCN Network, waiting synchronously to finish the task

    unique_hcn_namespace handle;
    wil::unique_cotaskmem_string errorRecord;
    std::wstring settings = LR"(
    {
  "NamespaceId": "F59CB073-28D0-41E7-AA5A-AE366B758471",
  "NamespaceName": "MyNATNamespace",
  "HostComputeNamespaceVersion": {
    "Major": 1,
    "Minor": 0
  }
})";

    GUID namespaceGuid;
    HRESULT result = CoCreateGuid(&namespaceGuid);

    result = HcnCreateNamespace(
        namespaceGuid,              // Unique ID
        settings.c_str(),      // Compute system settings document
        &handle,
        &errorRecord
    );
    if (FAILED(result))
    {
        printf("HcnCreateNamespace:: error= 0x%X \n", result);
        wprintf(L"HcnCreateNamespace: ERROR = %ws", errorRecord);

        THROW_HR(result);
    }

    result = HcnCloseNamespace(handle.get());
    if (FAILED(result))
    {
        // UnMarshal  the result Json
        THROW_HR(result);
    }
}


void CreateAndHotAddEndpoint()
{
    using unique_hcn_endpoint = wil::unique_any<
        HCN_ENDPOINT,
        decltype(&HcnCloseEndpoint),
        HcnCloseEndpoint>;

    using unique_hcn_network = wil::unique_any<
        HCN_NETWORK,
        decltype(&HcnCloseNetwork),
        HcnCloseNetwork>;

    unique_hcn_endpoint hcnendpoint;
    unique_hcn_network hcnnetwork;
    wil::unique_cotaskmem_string errorRecord;
    std::wstring settings = LR"(
{
    "SchemaVersion": {
        "Major": 2,
        "Minor": 0
    },
    "Name": "MyNATEndpoint",
    "Owner": "ME",
    "HostComputeNetwork": "71CD3B74-1A0A-447E-BF9E-3716C6652176",
    "IPAddress": "172.16.0.10",
    "PrefixLength": 12,
    "Flags": 0
})";


    //auto networkGuid = StringToGuid(L"71CD3B74-1A0A-447E-BF9E-3716C6652176");

    GUID networkGuid = {0x71CD3B74, 0x1A0A, 0x447E, {0xBF, 0x9E, 0x37, 0x16, 0xC6, 0x65, 0x21, 0x76}};

    GUID endpointGuid;
    HRESULT result = CoCreateGuid(&endpointGuid);

    result = HcnOpenNetwork(
        networkGuid,              // Unique ID
        &hcnnetwork,
        &errorRecord
    );
    if (FAILED(result))
    {
        printf("HcnOpenNetwork:: error= 0x%X \n", result);
        wprintf(L"HcnOpenNetwork: ERROR = %ws", errorRecord);

        // Failed to find network
        THROW_HR(result);
    }

    result = HcnCreateEndpoint(
        hcnnetwork.get(),
        endpointGuid,              // Unique ID
        settings.c_str(),      // Compute system settings document
        &hcnendpoint,
        &errorRecord
    );
    if (FAILED(result))
    {
        printf("HcnCreateEndpoint:: error= 0x%X \n", result);
        wprintf(L"HcnCreateEndpoint: ERROR = %ws", errorRecord);

        // Failed to create endpoint
        THROW_HR(result);
    }

    // Can use the sample from HCS API Spec on how to attach this endpoint
    // to the VM using AddNetworkAdapterToVm
    result = HcnCloseEndpoint(hcnendpoint.get());
    if (FAILED(result))
    {
        // UnMarshal  the result Json
        THROW_HR(result);
    }
}

int main()
{
    //CreateHcnNamespace();

    CreateAndHotAddEndpoint();

    //CreateHcnNetwork();

    return 0;
}