// DLL class definitions for access to USB HID devices
//
// (C) 2001-2014 Copyright Cleware GmbH
// All rights reserved
//
// History:
// 05.01.2001	ws	Initial coding
// 17.07.2001	ws	cleanup interface
// 10.12.2001	ws	cleanup interface again, basic class to hide implementation details
// 13.12.2001	ws	introduced versionNumber and virtual destructor
// 23.05.2002	ws	added switch access
// ...
// 03.02.2003	ws	added switch version 10 
// 04.08.2003	ws	added humidity 
// 21.01.2004	ws	fixed some humidity problems 
//		   2004	ws	added contact + io16
// 05.02.2005	ws	added ADC08-Support 
// 25.05.2013   ws	new controller support
// 15.05.2014	ws	4.3.0	added new controller support
// 08.07.2014	ws	4.3.1	added new controller support
// 08.08.2014	ws	4.3.2	added new IO16 support
// 02.09.2014	ws	4.3.3	fixed an old error when switching channel 8 of a Contact device using SetSwitch
// 05.07.2017	ws	4.5.5	Added support for KnoxBox V12, fixed small multiread bug
// 28.02.2018	ws	5.1.0	use USBaccessDevTypes.h,  added new Counter device to measure frequency



// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the USBACCESS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// USBACCESS_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.


#ifndef __USBACCESS_H__
#define __USBACCESS_H__

typedef int HANDLE ;

#ifdef USBACCESS_EXPORTS
#define USBACCESS_API __declspec(dllexport)
#else
#define USBACCESS_API __declspec(dllimport)
#endif

const int USBaccessVersion = 510 ;

class CUSBaccess {
	public:
		enum USBactions {		LEDs=0, EEwrite=1, EEread=2, Reset=3, KeepCalm=4, GetInfo=5, 
								StartMeasuring=6,		// USB-Humidity
								Configure=7,			// USB-IO16-V10, USB-Counter-V05
								RunPoint=10,			// USB-Encoder
								ContactWrite=11,	// 613er IO16
								ContactRead=12, 	// 613er IO16
								EEread4=13				// 613
								} ;
		enum USBInfoType {		OnlineTime=1, OnlineCount=2, ManualTime=3, ManualCount=4 } ;
		enum LED_IDs {			LED_0=0, LED_1=1, LED_2=2, LED_3=3 } ;
		enum COUNTER_IDs {		COUNTER_0=0, COUNTER_1=1 } ;
		enum SWITCH_IDs {		SWITCH_0=0x10, SWITCH_1=0x11, SWITCH_2=0x12, SWITCH_3=0x13,
								SWITCH_4=0x14, SWITCH_5=0x15, SWITCH_6=0x16, SWITCH_7=0x17,
								SWITCH_8=0x18, SWITCH_9=0x19, SWITCH_10=0x1a, SWITCH_11=0x1b,
								SWITCH_12=0x1c, SWITCH_13=0x1d, SWITCH_14=0x1e, SWITCH_15=0x1f
								} ;
		enum USBtype_enum {	
#include "USBaccessDevTypes.h"	
		} ;
	public:
		CUSBaccess() ;
		virtual ~CUSBaccess() ;		// maybe used as base class

		virtual int			OpenCleware() ;			// returns number of found Cleware devices
		virtual int			CloseCleware() ;		// close all Cleware devices
		virtual int			Recover(int devNum) ;	// try to find disconnected devices, returns true if succeeded
		virtual int			GetValue(int deviceNo, unsigned char *buf, int bufsize) ;
		virtual int			SetValue(int deviceNo, unsigned char *buf, int bufsize) ;
		virtual int			SetLED(int deviceNo, enum LED_IDs Led, int value) ;	// value: 0=off 7=medium 15=highlight
		virtual int			SetSwitch(int deviceNo, enum SWITCH_IDs Switch, int On) ;	//	On: 0=off, 1=on
		virtual int			GetSwitch(int deviceNo, enum SWITCH_IDs Switch) ;			//	On: 0=off, 1=on, -1=error
		virtual int			GetSeqSwitch(int deviceNo, enum SWITCH_IDs Switch, int seqNum) ;		//	On: 0=off, 1=on, -1=error
		virtual int			GetSwitchConfig(int deviceNo, int *switchCount, int *buttonAvailable) ;
		virtual int			GetTemperature(int deviceNo, double *Temperature, int *timeID) ;
		virtual float		GetTemperature(int deviceNo) ;
		virtual int			GetHumidity(int deviceNo, double *Humidity, int *timeID) ;
		virtual float		GetHumidity(int deviceNo) ;
		virtual int			SelectADC(int deviceNo, int subDevice) ;
		virtual float		GetADC(int deviceNo, int sequenceNumber, int subDevice) ;
		virtual int			ResetDevice(int deviceNo) ;
		virtual int			StartDevice(int deviceNo) ;	
		virtual int			CalmWatchdog(int deviceNo, int minutes, int minutes2restart) ;
		virtual int			GetVersion(int deviceNo) ;
		virtual int			GetUSBType(int deviceNo) ;
		virtual int			GetSerialNumber(int deviceNo) ;
		virtual int			GetDLLVersion() { return USBaccessVersion ; }
		virtual int			GetManualOnCount(int deviceNo) ;		// returns how often switch is manually turned on
		virtual int			GetManualOnTime(int deviceNo) ;			// returns how long (seconds) switch is manually turned on
		virtual int			GetOnlineOnCount(int deviceNo) ;		// returns how often switch is turned on by USB command
		virtual int			GetOnlineOnTime(int deviceNo) ;			// returns how long (seconds) switch is turned on by USB command
		virtual int			GetMultiSwitch(int deviceNo, unsigned long int *mask, unsigned long int *value, int seqNumber) ;
		virtual int			SetMultiSwitch(int deviceNo, unsigned long int value) ;
		virtual int			SetMultiConfig(int deviceNo, unsigned long int directions) ;
		virtual int			GetCounter(int deviceNo, enum COUNTER_IDs counterID) ;	// COUNTER_IDs ununsed until now
		virtual int			SetCounter(int deviceNo, int counter, enum COUNTER_IDs counterID) ;	//  -1=error, COUNTER_IDs ununsed until now
		virtual int			SyncDevice(int deviceNo, unsigned long int mask) ;
		virtual int			GetHWversion(int deviceNo) ;	// return HWversion (0 for pre 2014 designed devices, 13 for new devices)
		virtual int			IsAmpel(int deviceNo) ;	// return true if this is a traffic light device
		virtual int			IOX(int deviceNo, int addr, int data) ;		// for internal use only, wrong usage may destroy device	
		virtual int			IsIdeTec(int deviceNo) ;			// return true if watchdog inverted
		virtual int			GetFrequency(int deviceNo, unsigned long int *counter, int subDevice) ;
		virtual void		DebugWrite(char *s) ;
		virtual void		DebugWrite(char *f, int a1) ;
		virtual void		DebugWrite(char *f, int a1, int a2) ;
		virtual void		DebugWrite(char *f, int a1, int a2, int a3) ;
		virtual void		DebugWrite(char *f, int a1, int a2, int a3, int a4) ;
		virtual void		Sleep(int ms) { usleep(ms * 1000) ; }	// for Linux
	} ;

#endif // __USBACCESS_H__
