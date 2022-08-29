#include "driver.h"

UNICODE_STRING name, link;

VOID unload_drv(PDRIVER_OBJECT obj) {
	NTSTATUS Status = { };

	Status = IoDeleteSymbolicLink(&link);

	if (!NT_SUCCESS(Status))
		return;

	IoDeleteDevice(obj->DeviceObject);
}

nt controller(PDEVICE_OBJECT obj, PIRP irp) {
	UNREFERENCED_PARAMETER(obj);

	nt status = { };
	ULONG bytes = { };
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);

	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
	ULONG input_lenght = stack->Parameters.DeviceIoControl.InputBufferLength;

	if (code == code_virtual) {
		if (input_lenght == sizeof(t_virtual)) {
			s_virtual x = (s_virtual)(irp->AssociatedIrp.SystemBuffer);

			status = virtual_memory(x);
			bytes = sizeof(t_virtual);
		}
		else {
			status = STATUS_INFO_LENGTH_MISMATCH;
			bytes = NULL;
		}
	}
	else if (code == code_physical) {
		if (input_lenght == sizeof(t_physical)) {
			s_physical x = (s_physical)(irp->AssociatedIrp.SystemBuffer);

			status = physcial_memory(x);
			bytes = sizeof(t_physical);
		}
		else {
			status = STATUS_INFO_LENGTH_MISMATCH;
			bytes = NULL;
		}
	}
	else if (code == code_image) {
		if (input_lenght == sizeof(t_image)) {
			s_image x = (s_image)(irp->AssociatedIrp.SystemBuffer);

			status = image(x);
			bytes = sizeof(t_image);
		}
		else {
			status = STATUS_INFO_LENGTH_MISMATCH;
			bytes = NULL;
		}
	}

	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytes;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return status;
}

nt is_unsupported(PDEVICE_OBJECT obj, PIRP irp) {
	UNREFERENCED_PARAMETER(obj);

	irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return irp->IoStatus.Status;
}

nt dispatch_handler(PDEVICE_OBJECT obj, PIRP irp)
{
	UNREFERENCED_PARAMETER(obj);

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);

	switch (stack->MajorFunction) {
	case IRP_MJ_CREATE:
		break;
	case IRP_MJ_CLOSE:
		break;
	default:
		break;
	}

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return irp->IoStatus.Status;
}

nt start_driver(PDRIVER_OBJECT obj, PUNICODE_STRING path) {
	UNREFERENCED_PARAMETER(path);

	nt _nt = NULL;
	PDEVICE_OBJECT DeviceObject = NULL;

	RtlInitUnicodeString(&name, L"\\Device\\pymodule");
	RtlInitUnicodeString(&link, L"\\DosDevices\\pymodule");

	_nt = IoCreateDevice(obj, 0, &name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

	if (!NT_SUCCESS(_nt))
		return STATUS_UNSUCCESSFUL;

	_nt = IoCreateSymbolicLink(&link, &name);

	if (!NT_SUCCESS(_nt))
		return STATUS_UNSUCCESSFUL;

	for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
		obj->MajorFunction[i] = &is_unsupported;

	DeviceObject->Flags |= DO_BUFFERED_IO;

	obj->MajorFunction[IRP_MJ_CREATE] = &dispatch_handler;
	obj->MajorFunction[IRP_MJ_CLOSE] = &dispatch_handler;
	obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &controller;
	obj->DriverUnload = &unload_drv;

	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}

nt DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	return IoCreateDriver(NULL, &start_driver);
}