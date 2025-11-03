#ifndef _SWS_HEADERS__H
#define _SWS_HEADERS__H

typedef BOOLEAN Bool;
typedef UCHAR UChar;
typedef USHORT Uint2B;
typedef ULONG Uint4B;
typedef ULONGLONG Uint8B;
typedef ULONG_PTR Ptr64;

typedef struct _SWS_NDIS_FILTER_BLOCK SWS_NDIS_FILTER_BLOCK, *PSWS_NDIS_FILTER_BLOCK;
typedef struct _SWS_NDIS_MINIPORT_BLOCK SWS_NDIS_MINIPORT_BLOCK, *PSWS_NDIS_MINIPORT_BLOCK;

typedef int(__fastcall* TcpOffloadReceiveReturnHandler_f)(void*, NET_BUFFER_LIST*);

typedef struct _SWS_REFERENCE_EX
{
	KSPIN_LOCK SpinLock;
	Uint2B ReferenceCount;
	UChar Closing;
	UChar ZeroBased;
	Ptr64 RefCountTracker;

}SWS_REFERENCE_EX, *PSWS_REFERENCE_EX;

typedef struct _Rtl_KString
{
	Uint2B Length;
	Uint2B MaximumLength;
	PWCHAR Buffer;

}Rtl_KString, * PRtl_KString;

typedef struct _KRef__NDIS_BIND_FILTER_DRIVER
{
	struct KRef__NDIS_BIND_FILTER_DRIVER__KRefHolder* RefHolder;
}KRef__NDIS_BIND_FILTER_DRIVER, * PKRef__NDIS_BIND_FILTER_DRIVER;


typedef struct _SWS_NDIS_FILTER_BLOCK
{
	NDIS_OBJECT_HEADER Header;
	struct _SWS_NDIS_FILTER_BLOCK* NextFilter;
	struct _SWS_NDIS_FILTER_BLOCK* FilterDriver;
	Ptr64 FilterModuleContext;
	struct SWS_NDIS_MINIPORT_BLOCK* Miniport;
	Ptr64 FilterInstanceName;
	UNICODE_STRING* FilterFriendlyName;

}SWS_NDIS_FILTER_BLOCK_SWS, * PSWS_NDIS_FILTER_BLOCK;


typedef struct _SWS_NDIS_FILTER_DRIVER_BLOCK
{
	NDIS_OBJECT_HEADER 						Header;
	struct _SWS_NDIS_FILTER_DRIVER_BLOCK*	NextFilterDriver;
	DRIVER_OBJECT*							DriverObject;
	struct _SWS_NDIS_FILTER_BLOCK*			FilterQueue;
	Ptr64 									FilterDriverContext;
	KSPIN_LOCK 								Lock;
	Uint4B 									Flags;
	LIST_ENTRY 								DeviceList;
	SWS_REFERENCE_EX 						Ref;
	NDIS_FILTER_DRIVER_CHARACTERISTICS		DefaultFilterCharacteristics;
	TcpOffloadReceiveReturnHandler_f		TcpOffloadReceiveReturnHandler;
	KRef__NDIS_BIND_FILTER_DRIVER 			Bind; //Ptr64
	UNICODE_STRING							ImageName;

}SWS_NDIS_FILTER_DRIVER_BLOCK, * PSWS_NDIS_FILTER_DRIVER_BLOCK;

typedef struct _SWS_NDIS_BIND_FILTER_DRIVER
{
	Bool DriverReady;
	Bool NeedsBindCompleteEven;
	LIST_ENTRY BindLinks;
	GUID Guid;
	Uint4B FilterBindFlags;
	Ptr64 FilterClass;
	struct _SWS_NDIS_FILTER_DRIVER_BLOCK* RunningDriver;

}SWS_NDIS_BIND_FILTER_DRIVER, * PSWS_NDIS_BIND_FILTER_DRIVER;

typedef struct _SWS_NDIS_PCW_DATA_BLOCK
{
	Uint4B DatapathEventReferences[26];
	Uint4B DatapathCycleReferences[13];
	struct _SWS_NDIS_PCW_DATA_BLOCK* Next;
	Uint4B ReferenceCount;
	struct _SWS_NDIS_MINIPORT_BLOCK* Miniport;
	Uint4B TotalInstanceId;
	UNICODE_STRING TotalInstanceName;

}SWS_NDIS_PCW_DATA_BLOCK, * PSWS_NDIS_PCW_DATA_BLOCK;

typedef struct _SWS_NDIS_MINIPORT_BLOCK
{
	NDIS_OBJECT_HEADER Header;
	struct _SWS_NDIS_MINIPORT_BLOCK* NextMiniport;
	struct _SWS_NDIS_MINIPORT_BLOCK* BaseMiniport;
	PVOID MiniportAdapterContext;
	UNICODE_STRING Reserved4;
	UChar MajorNdisVersion;
	UChar MinorNdisVersion;
	struct _SWS_NDIS_PCW_DATA_BLOCK* PcwDataBlock;

}SWS_NDIS_MINIPORT_BLOCK, * PSWS_NDIS_MINIPORT_BLOCK;

typedef struct _KRef__NDIS_BIND_FILTER_DRIVER__KRefHolder
{
	SWS_NDIS_BIND_FILTER_DRIVER _t;
	Uint4B RefCount;

} KRef__NDIS_BIND_FILTER_DRIVER__KRefHolder, * PKRef__NDIS_BIND_FILTER_DRIVER__KRefHolder;


//https://github.com/winsiderss/systeminformer/blob/78124af1f54187762af94ebc9aab9a9818a53adf/phnt/include/ntexapi.h#L2089
typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,                                 // q: SYSTEM_BASIC_INFORMATION
	SystemProcessorInformation,                             // q: SYSTEM_PROCESSOR_INFORMATION
	SystemPerformanceInformation,                           // q: SYSTEM_PERFORMANCE_INFORMATION
	SystemTimeOfDayInformation,                             // q: SYSTEM_TIMEOFDAY_INFORMATION
	SystemPathInformation,                                  // q: not implemented
	SystemProcessInformation,                               // q: SYSTEM_PROCESS_INFORMATION
	SystemCallCountInformation,                             // q: SYSTEM_CALL_COUNT_INFORMATION
	SystemDeviceInformation,                                // q: SYSTEM_DEVICE_INFORMATION
	SystemProcessorPerformanceInformation,                  // q: SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION (EX in: USHORT ProcessorGroup)
	SystemFlagsInformation,                                 // qs: SYSTEM_FLAGS_INFORMATION
	SystemCallTimeInformation,                              // q: SYSTEM_CALL_TIME_INFORMATION // not implemented // 10
	SystemModuleInformation                                // q: RTL_PROCESS_MODULES
}SYSTEM_INFORMATION_CLASS;

typedef NTSTATUS(__fastcall* NdisMSetInterfaceCompartment_f)(SWS_NDIS_MINIPORT_BLOCK* miniportBlock, const GUID* compartmentGuid);

NdisMSetInterfaceCompartment_f SwsFindNdisMSetInterfaceCompartment();

#endif
