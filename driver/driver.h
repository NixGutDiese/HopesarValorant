#pragma once
#include "util.h"
#include "physical.h"

nt virtual_memory(s_virtual x) {
	if (x->security_code != code_security)
		return STATUS_UNSUCCESSFUL;

	if (!x->process_id)
		return STATUS_UNSUCCESSFUL;

	SIZE_T virtual_bytes = NULL;
	PEPROCESS process = NULL;
	PsLookupProcessByProcessId((HANDLE)x->process_id, &process);

	if (x->virtual_mode == virtual_read) {
		MmCopyVirtualMemory(process, (PVOID)x->address, this_process, (PVOID)x->buffer, x->size, UserMode, &virtual_bytes);
		return STATUS_SUCCESS;
	}
	else if (x->virtual_mode == virtual_write) {
		MmCopyVirtualMemory(this_process, (PVOID)x->buffer, process, (PVOID)x->address, x->size, UserMode, &virtual_bytes);
		return STATUS_SUCCESS;
	}
	ObfDereferenceObject(process);

	return STATUS_UNSUCCESSFUL;
}

nt physcial_memory(s_physical x) {
	if (x->security_code != code_security)
		return STATUS_UNSUCCESSFUL;

	if (!x->process_id) 
		return STATUS_UNSUCCESSFUL;

	PEPROCESS process = NULL;
	PsLookupProcessByProcessId((HANDLE)x->process_id, &process);
	if (!process)
		return STATUS_UNSUCCESSFUL;

	ULONGLONG process_base = get_process_cr3(process);
	ObDereferenceObject(process);

	SIZE_T this_offset = NULL;
	SIZE_T total_size = x->size;

	INT64 physical_address = translate_linear(process_base, (ULONG64)x->address + this_offset);
	if (!physical_address)
		return STATUS_UNSUCCESSFUL;

	ULONG64 final_size = find_min(PAGE_SIZE - (physical_address & 0xFFF), total_size);
	SIZE_T bytes_trough = NULL;

	if (x->physical_mode == physical_read) {
		read_physical(PVOID(physical_address), (PVOID)((ULONG64)x->buffer + this_offset), final_size, &bytes_trough);
		return STATUS_SUCCESS;
	}
	else if (x->physical_mode == physical_write) {
		write_physical(PVOID(physical_address), (PVOID)((ULONG64)x->buffer + this_offset), final_size, &bytes_trough);
		return STATUS_SUCCESS;
	}
	
	return STATUS_UNSUCCESSFUL;
}

nt image(s_image x) {
	if (x->security_code != code_security)
		return STATUS_UNSUCCESSFUL;

	if (!x->process_id)
		return STATUS_UNSUCCESSFUL;

	PEPROCESS process = NULL;
	PsLookupProcessByProcessId((HANDLE)x->process_id, &process);
	if (!process)
		return STATUS_UNSUCCESSFUL;

	ULONGLONG image_base = (ULONGLONG)PsGetProcessSectionBaseAddress(process);
	if (!image_base)
		return STATUS_UNSUCCESSFUL;

	RtlCopyMemory(x->address, &image_base, sizeof(image_base));
	ObDereferenceObject(process);

	return STATUS_UNSUCCESSFUL;
}