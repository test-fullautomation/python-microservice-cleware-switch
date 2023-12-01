#ifndef __USBACCESS_L_H__
#define __USBACCESS_L_H__
#include "USBaccess.h"

#ifdef __cplusplus
// enum USBactions {		LEDs=0, EEwrite=1, EEread=2, Reset=3, KeepCalm=4, GetInfo=5, 
// 								StartMeasuring=6,		// USB-Humidity
// 								Configure=7,			// USB-IO16-V10, USB-Counter-V05
// 								RunPoint=10,			// USB-Encoder
// 								ContactWrite=11,	// 613er IO16
// 								ContactRead=12, 	// 613er IO16
// 								EEread4=13				// 613
// 								} ;
		enum USBInfoType {		OnlineTime=1, OnlineCount=2, ManualTime=3, ManualCount=4 } ;
		enum LED_IDs {			LED_0=0, LED_1=1, LED_2=2, LED_3=3 } ;
		enum COUNTER_IDs {		COUNTER_0=0, COUNTER_1=1 } ;
		enum SWITCH_IDs {		SWITCH_0=0x10, SWITCH_1=0x11, SWITCH_2=0x12, SWITCH_3=0x13,
								SWITCH_4=0x14, SWITCH_5=0x15, SWITCH_6=0x16, SWITCH_7=0x17,
								SWITCH_8=0x18, SWITCH_9=0x19, SWITCH_10=0x1a, SWITCH_11=0x1b,
								SWITCH_12=0x1c, SWITCH_13=0x1d, SWITCH_14=0x1e, SWITCH_15=0x1f
								} ;
extern "C" {
#endif
	CUSBaccess*		FCWInitObject(void);
	void 			FCWUnInitObject(CUSBaccess* obj);
	int 			FCWOpenCleware(CUSBaccess* obj);
	int 			FCWCloseCleware(CUSBaccess* obj);
	int             FCWGetTheRealDeviceNum(CUSBaccess* obj, int deviceNo);
	//int 			FCWRecover(CUSBaccess* obj, int deviceNo);
	//void*			FCWGetHandle(CUSBaccess* obj, int deviceNo);
	//int 			FCWGetValue(CUSBaccess* obj, int deviceNo, unsigned char* buf, int bufsize);
	//int 			FCWSetValue(CUSBaccess* obj, int deviceNo, unsigned char* buf, int bufsize);
	//int 			FCWSetLED(CUSBaccess* obj, int deviceNo, enum FCWLED_IDs Led, int value);	// value: 0=off 7=medium 15=highlight
	int 			FCWSetSwitch(CUSBaccess* obj, int deviceNo, enum SWITCH_IDs Switch, int On);	//	On: 0=off, 1=on
	int 			FCWGetSwitch(CUSBaccess* obj, int deviceNo, enum SWITCH_IDs Switch);			//	On: 0=off, 1=on, -1=error
	//int 			FCWGetSeqSwitch(CUSBaccess* obj, int deviceNo, enum FCWSWITCH_IDs Switch, int seqNum);			//	On: 0=off, 1=on, -1=error
	//int 			FCWGetSwitchConfig(CUSBaccess* obj, int deviceNo, int* switchCount, int* buttonAvailable);
	//int 			FCWGetTemperature(CUSBaccess* obj, int deviceNo, double* Temperature, int* timeID);
	//float 		FCWDGetTemperature(CUSBaccess* obj, int deviceNo);
	//int 			FCWGetHumidity(CUSBaccess* obj, int deviceNo, double* Humidity, int* timeID);
	//float 		FCWDGetHumidity(CUSBaccess* obj, int deviceNo);
	//int 			FCWSelectADC(CUSBaccess* obj, int deviceNo, int subDevice);
	//float 		FCWGetADC(CUSBaccess* obj, int deviceNo, int sequenceNumber, int subDevice);
	//int 			FCWResetDevice(CUSBaccess* obj, int deviceNo);
	//int 			FCWStartDevice(CUSBaccess* obj, int deviceNo);
	//int 			FCWCalmWatchdog(CUSBaccess* obj, int deviceNo, int minutes, int minutes2restart);
	//int 			FCWGetVersion(CUSBaccess* obj, int deviceNo);
	//int 			FCWGetUSBType(CUSBaccess* obj, int deviceNo);
	int 			FCWGetSerialNumber(CUSBaccess* obj, int deviceNo);
	int*			FCWGetAllSwitchState(CUSBaccess* obj, int deviceNo);
	//int 			FCWGetDLLVersion(void);
	//int 			FCWGetManualOnCount(CUSBaccess* obj, int deviceNo);
	//int 			FCWGetManualOnTime(CUSBaccess* obj, int deviceNo);
	//int 			FCWGetOnlineOnCount(CUSBaccess* obj, int deviceNo);
	//int 			FCWGetOnlineOnTime(CUSBaccess* obj, int deviceNo);
	//int 			FCWGetMultiSwitch(CUSBaccess* obj, int deviceNo, unsigned long int* mask, unsigned long int* value, int seqNumber);
	//int 			FCWSetMultiSwitch(CUSBaccess* obj, int deviceNo, unsigned long int value);
	//int 			FCWSetMultiConfig(CUSBaccess* obj, int deviceNo, unsigned long int directions);
	//int 			FCWGetCounter(CUSBaccess* obj, int deviceNo, int counter);
	//int 			FCWSyncDevice(CUSBaccess* obj, int deviceNo, unsigned long int mask);
	//int 			FCWGetMultiConfig(CUSBaccess* obj, int deviceNo);
	//int 			FCWIsAmpel(CUSBaccess* obj, int deviceNo);
	//int 			FCWIsLuminus(CUSBaccess* obj, int deviceNo);
	//int 			FCWIsAlarm(CUSBaccess* obj, int deviceNo);
	//int 			FCWIsCutter(CUSBaccess* obj, int deviceNo);
	//int 			FCWIsSolidState(CUSBaccess* obj, int deviceNo);
	//int 			FCWIsWatchdogInvert(CUSBaccess* obj, int deviceNo);
	//int 			FCWGetHWversion(CUSBaccess* obj, int deviceNo);
	//int 			FCWIOX(CUSBaccess* obj, int deviceNo, int addr, int data);		// for internal use only, wrong usage may destroy device
	//int 			FCWSetTempOffset(CUSBaccess* obj, int deviceNo, double Sollwert, double IstWert); // returns 1 if ok or 0 in case of an error
	//int 			FCWGetFrequency(CUSBaccess* obj, int deviceNo, unsigned long int* counter, int subDevice); // returns frequency if ok or -1 in case of an error

#ifdef __cplusplus
}
#endif

#endif /* __MATHER_H__ */
