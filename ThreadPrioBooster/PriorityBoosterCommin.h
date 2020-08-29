#pragma once

//Controlcode
#define PRIORITY_BOOSTER_DEVICE 0x8000
#define IOCTL_PRIORITY_BOOSTER_SET_PRIORITY CTL_CODE(PRIORITY_BOOSTER_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

struct ThreadData
{
	ULONG ThreadId; //ThreadID == 32 Bit // ULONG is defined in Kernel and Usermode
	int Priority;
};