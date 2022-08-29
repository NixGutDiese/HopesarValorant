#pragma once
#include <ntifs.h>

INT32 GetUserDirectoryTableBaseOffset()
{
	RTL_OSVERSIONINFOW ver = { 0 };
	RtlGetVersion(&ver);
	switch (ver.dwBuildNumber)
	{
	case win_1803:
		return 0x0278;
		break;
	case win_1809:
		return 0x0278;
		break;
	case win_1903:
		return 0x0280;
		break;
	case win_1909:
		return 0x0280;
		break;
	case win_2004:
		return 0x0388;
		break;
	case win_20H2:
		return 0x0388;
		break;
	case win_21H1:
		return 0x0388;
		break;
	default:
		return 0x0388;
	}
}

UINT64 get_process_cr3(const PEPROCESS pProcess)
{
	PUCHAR process = (PUCHAR)pProcess;
	ULONG_PTR process_dirbase = *(PULONG_PTR)(process + 0x28); //dirbase x64, 32bit is 0x18
	if (process_dirbase == 0)
	{
		INT32 UserDirOffset = GetUserDirectoryTableBaseOffset();
		ULONG_PTR process_userdirbase = *(PULONG_PTR)(process + UserDirOffset);
		return process_userdirbase;
	}
	return process_dirbase;
}

#define PAGE_OFFSET_SIZE 12
static const UINT64 PMASK = (~0xfull << 8) & 0xfffffffffull;

NTSTATUS read_physical(PVOID TargetAddress, PVOID lpBuffer, SIZE_T Size, SIZE_T* BytesRead)
{
	MM_COPY_ADDRESS AddrToRead = { 0 };
	AddrToRead.PhysicalAddress.QuadPart = (LONGLONG)TargetAddress;
	return MmCopyMemory(lpBuffer, AddrToRead, Size, MM_COPY_MEMORY_PHYSICAL, BytesRead);
}
UINT64 translate_linear(UINT64 directoryTableBase, UINT64 virtualAddress)
{
	directoryTableBase &= ~0xf;

	UINT64 pageOffset = virtualAddress & ~(~0ul << PAGE_OFFSET_SIZE);
	UINT64 pte = ((virtualAddress >> 12) & (0x1ffll));
	UINT64 pt = ((virtualAddress >> 21) & (0x1ffll));
	UINT64 pd = ((virtualAddress >> 30) & (0x1ffll));
	UINT64 pdp = ((virtualAddress >> 39) & (0x1ffll));

	SIZE_T readsize = 0;
	UINT64 pdpe = 0;
	read_physical(PVOID(directoryTableBase + 8 * pdp), &pdpe, sizeof(pdpe), &readsize);
	if (~pdpe & 1)
		return 0;

	UINT64 pde = 0;
	read_physical(PVOID((pdpe & PMASK) + 8 * pd), &pde, sizeof(pde), &readsize);
	if (~pde & 1)
		return 0;

	/* 1GB large page, use pde's 12-34 bits */
	if (pde & 0x80)
		return (pde & (~0ull << 42 >> 12)) + (virtualAddress & ~(~0ull << 30));

	UINT64 pteAddr = 0;
	read_physical(PVOID((pde & PMASK) + 8 * pt), &pteAddr, sizeof(pteAddr), &readsize);
	if (~pteAddr & 1)
		return 0;

	/* 2MB large page */
	if (pteAddr & 0x80)
		return (pteAddr & PMASK) + (virtualAddress & ~(~0ull << 21));

	virtualAddress = 0;
	read_physical(PVOID((pteAddr & PMASK) + 8 * pte), &virtualAddress, sizeof(virtualAddress), &readsize);
	virtualAddress &= PMASK;

	if (!virtualAddress)
		return 0;

	return virtualAddress + pageOffset;
}
NTSTATUS write_physical(PVOID TargetAddress, PVOID lpBuffer, SIZE_T Size, SIZE_T* BytesWritten)
{
	if (!TargetAddress)
		return STATUS_UNSUCCESSFUL;

	PHYSICAL_ADDRESS AddrToWrite = { 0 };
	AddrToWrite.QuadPart = LONGLONG(TargetAddress);

	PVOID pmapped_mem = MmMapIoSpaceEx(AddrToWrite, Size, PAGE_READWRITE);

	if (!pmapped_mem)
		return STATUS_UNSUCCESSFUL;

	memcpy(pmapped_mem, lpBuffer, Size);

	*BytesWritten = Size;
	MmUnmapIoSpace(pmapped_mem, Size);
	return STATUS_SUCCESS;
}

ULONG64 find_min(INT32 g, SIZE_T f) {
	INT32 h = (INT32)f;
	ULONG64 result = 0;

	result = (((g) < (h)) ? (g) : (h));

	return result;
}