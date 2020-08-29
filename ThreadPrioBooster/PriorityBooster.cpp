#include <ntifs.h>
#include <ntddk.h>
#include "PriorityBoosterCommin.h"

//Also claculates the lengh of the string statically at compile time
UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\PriorityBooster");
UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\PriorityBooster");


void UnloadRoutine(_In_ PDRIVER_OBJECT DriverObject) {
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);

	KdPrint(("[DEBUG] DeeGee ThreadPriobooster - Driver unloaded successfully"));
}

//input Pointer to device object and pointer to I/O Request Packet (Object where the request info in stored)
_Use_decl_annotations_
NTSTATUS PriorityBoosterCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS PriorityBoosterDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	//get IO_STACK_LOCATION
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode){
	case IOCTL_PRIORITY_BOOSTER_SET_PRIORITY:
	{
		//do work

		//Check if received buffer is large enough for ThreadData
		if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ThreadData)) {
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		auto data = (ThreadData*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
		if (data == nullptr) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		if (data->Priority < 1 || data->Priority > 31) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		PETHREAD Thread;
		status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &Thread);
		if (!NT_SUCCESS(status))
			break;

		KeSetPriorityThread((PKTHREAD)Thread, data->Priority);
		ObDereferenceObject(Thread);
		KdPrint(("Thread Priority change for %d to %d succeeded!\n", data->ThreadId, data->Priority));
		break;
	}

	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

extern "C" NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	//Get windows version
	RTL_OSVERSIONINFOW info = { sizeof(info) };
	RtlGetVersion(&info);
	KdPrint(("[DEBUG] DeeGee ThreadPriobooster - Windows Version: %d.%d.%d\n", info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber));

	DriverObject->DriverUnload = UnloadRoutine;
	KdPrint(("[DEBUG] DeeGee ThreadPriobooster - driver initialized successfully\n"));

	//Dispatch routines
	DriverObject->MajorFunction[IRP_MJ_CREATE] = PriorityBoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = PriorityBoosterCreateClose;
	//Driver Client communication
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PriorityBoosterDeviceControl;

	KdPrint(("[DEBUG] DeeGee ThreadPriobooster - dispatch routines initialized successfully\n"));

	//Creating device object
	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[DEBUG] DeeGee ThreadPriobooster - Failed to create device onject (0x%08X)\n", status));
		return status;
	}

	KdPrint(("[DEBUG] DeeGee ThreadPriobooster - creating device object successfully\n"));

	//Creating symbolic link
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[DEBUG] DeeGee ThreadPriobooster - Failed to create symbolic link (0x%08X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}

	KdPrint(("[DEBUG] DeeGee ThreadPriobooster - creating symbolic link successfully\n"));


	return STATUS_SUCCESS;
}