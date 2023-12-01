/**
    USBAccessLinux.cpp 
    Purpose: Wrapper the CUSBaccess object methods to export in a shared library.

    @author Cuong Nguyen
    @version 0.1 13/09/2019
*/
#include <unistd.h>
#include "USBaccess.h"
#include "USBAccessLinux.h"



CUSBaccess* FCWInitObject(void)
{
	CUSBaccess* CWusb = new CUSBaccess;
	return CWusb;
}
void FCWUnInitObject(CUSBaccess* obj)
{
	if(obj)
	{
		delete obj;
	}
}

int FCWOpenCleware(CUSBaccess* obj)
{
	return obj->OpenCleware();
}

int FCWCloseCleware(CUSBaccess* obj)
{
	return obj->CloseCleware();
}

int FCWSetSwitch(CUSBaccess* obj, int deviceNo, enum SWITCH_IDs Switch, int On)	//	On: 0=off, 1=on
{
    deviceNo = FCWGetTheRealDeviceNum(obj, deviceNo);
	return obj->SetSwitch(deviceNo, (CUSBaccess::SWITCH_IDs)Switch, On);
}

int FCWGetTheRealDeviceNum(CUSBaccess* obj, int deviceNo)
{
    int iRes = deviceNo;
    int nDevices = FCWOpenCleware(obj);
    for(int i=0; i < nDevices; i++)
    {
        int nSerial = FCWGetSerialNumber(obj, i);
        if(nSerial == deviceNo)
        {
            iRes = i;
            break;
        }
    }
    return iRes;
}

int FCWGetSwitch(CUSBaccess* obj, int deviceNo, enum SWITCH_IDs Switch)			//	On: 0=off, 1=on, -1=error
{
    deviceNo = FCWGetTheRealDeviceNum(obj, deviceNo);
	return	obj->GetSwitch(deviceNo, (CUSBaccess::SWITCH_IDs)Switch);
}

int FCWGetSerialNumber(CUSBaccess* obj, int deviceNo)
{
    return obj->GetSerialNumber(deviceNo);
}

int* FCWGetAllSwitchState(CUSBaccess* obj, int deviceNo)
{
    int* array = new int[CUSBaccess::SWITCH_8 - CUSBaccess::SWITCH_0]; // Creating a sample int array
	for (int i = 0; i < (CUSBaccess::SWITCH_8 - CUSBaccess::SWITCH_0); ++i) {
	    array[i] = obj->GetSwitch(deviceNo, (CUSBaccess::SWITCH_IDs)(i + CUSBaccess::SWITCH_0));
	}
    return array;
}