#pragma once
#include <ntifs.h>
#include <winternl.h>

#define nt NTSTATUS
#define pvoid PVOID
#define ext_c extern "C"
#define win_1803 17134
#define win_1809 17763
#define win_1903 18362
#define win_1909 18363
#define win_2004 19041
#define win_20H2 19569
#define win_21H1 20180
#define physical_read 0x27b
#define physical_write 0xdfd
#define virtual_read 0xfc6
#define virtual_write 0x359
#define this_process IoGetCurrentProcess()
#define ustring UNICODE_STRING

ext_c nt NTAPI MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
ext_c nt NTAPI IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
ext_c pvoid NTAPI PsGetProcessSectionBaseAddress(PEPROCESS Process);

#define code_virtual CTL_CODE(FILE_DEVICE_UNKNOWN, 0x269, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_physical CTL_CODE(FILE_DEVICE_UNKNOWN, 0x472, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_image CTL_CODE(FILE_DEVICE_UNKNOWN, 0xfee, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_security 0xcabd5e5

typedef struct t_virtual {
	INT32 security_code;
	INT32 process_id;
	INT32 virtual_mode;
	ULONGLONG address;
	ULONGLONG buffer;
	ULONGLONG size;
} e_virtual, * s_virtual;

typedef struct t_physical {
	INT32 security_code;
	INT32 process_id;
	INT32 physical_mode;
	ULONGLONG address;
	ULONGLONG buffer;
	ULONGLONG size;
} e_physical, * s_physical;

typedef struct t_image {
	INT32 security_code;
	INT32 process_id;
	ULONGLONG* address;
} e_image, * s_image;