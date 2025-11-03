#include "precomp.h"

extern PVOID RtlFindExportedRoutineByName(PVOID DllBase, PCHAR RoutineName);
extern NTSTATUS ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{ // Information Class 11
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
    ULONG ModulesCount;
    RTL_PROCESS_MODULE_INFORMATION Modules[ANYSIZE_ARRAY];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

BOOLEAN StringsEqual(const char* str1, const char* str2, size_t maxLen)
{
    for (size_t i = 0; i < maxLen; ++i)
    {
        if (str1[i] != str2[i])
            return FALSE;
        if (str1[i] == '\0')
            return TRUE;
    }
    return TRUE;
}

NTSTATUS GetSystemModules(PRTL_PROCESS_MODULES* Modules)
{
    ULONG ReturnLength = 0;
    NTSTATUS ntStatus = ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &ReturnLength);
    if (ntStatus != STATUS_INFO_LENGTH_MISMATCH)
    {
        return ntStatus;
    }
    else if (ReturnLength == 0)
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    PRTL_PROCESS_MODULES pModules = (PRTL_PROCESS_MODULES)ExAllocatePoolUninitialized(PagedPool, ReturnLength, '0sws');
    if (pModules == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    ntStatus = ZwQuerySystemInformation(SystemModuleInformation, pModules, ReturnLength, NULL);
    if (!NT_SUCCESS(ntStatus))
    {
        ExFreePoolWithTag(pModules, '0sws');
        return ntStatus;
    }

    *Modules = pModules;

    return STATUS_SUCCESS;
}

NTSTATUS GetModuleImageBase(const CHAR* ModuleName, PVOID* ImageBase)
{
    CHAR* FileName = NULL;

    PRTL_PROCESS_MODULES ModulesInfo = NULL;
    NTSTATUS ntStatus = GetSystemModules(&ModulesInfo);
    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

    ntStatus = STATUS_NOT_FOUND;

    for (ULONG i = 0; i < ModulesInfo->ModulesCount; ++i)
    {
        FileName = (CHAR*)ModulesInfo->Modules[i].FullPathName[ModulesInfo->Modules[i].OffsetToFileName];

        if (StringsEqual(ModuleName, FileName, 256) == TRUE)
        {
            *ImageBase = ModulesInfo->Modules[i].ImageBase;
            ntStatus = STATUS_SUCCESS;
            break;
        }
    }

    ExFreePoolWithTag(ModulesInfo, '0sws');

    return ntStatus;
}

PVOID SwsGetModuleRoutineByName(const CHAR* ModuleName, CHAR* RoutineName)
{
    PVOID ImageBase = NULL;
    NTSTATUS ntStatus = GetModuleImageBase(ModuleName, &ImageBase);
    if (!NT_SUCCESS(ntStatus))
    {
        return NULL;
    }

    return RtlFindExportedRoutineByName(ImageBase, RoutineName);
}

NdisMSetInterfaceCompartment_f SwsFindNdisMSetInterfaceCompartment()
{
    return (NdisMSetInterfaceCompartment_f)SwsGetModuleRoutineByName("ndis.sys", "NdisMSetInterfaceCompartment");
}