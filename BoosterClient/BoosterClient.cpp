#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include "..\ThreadPrioBooster\PriorityBoosterCommin.h"

int Error(const char* message) {
	printf("%s (error=%d)\n", message, GetLastError());
	return 1;
}

int main(int argc, const char* argv[]){
	if (argc < 3)
	{
		printf("Usage: Booster <threadid> <priority>\n");
		return 0;
	}

	//Open a handle to the device
	HANDLE hDevice = CreateFile(L"\\\\.\\PriorityBooster", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE)
		return Error("Failed to open device");
	
	//Thread data structure
	ThreadData data;
	data.ThreadId = atoi(argv[1]);
	data.Priority = atoi(argv[2]);

	//Call DeviceIoControl
	DWORD returned;
	BOOL successs = DeviceIoControl(hDevice, IOCTL_PRIORITY_BOOSTER_SET_PRIORITY, &data, sizeof(data), nullptr, 0, &returned, nullptr);
	if (successs)
	{
		printf("Priority change succeeded!\n");
	}
	else
	{
		Error("Priority change failed!");
	}

	CloseHandle(hDevice);
}

