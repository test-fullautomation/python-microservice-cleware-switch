// Basic class definitions for access to USB HID devices
//
// (C) 2001 Copyright Cleware GmbH
// All rights reserved
//
// History:
// 05.01.01	ws	Initial coding
// 17.07.01	ws	cleanup interface
// 12.05.06	ws	lift to Version 3.3.0
// 25.05.13	ws	support for new controller devices
// 08.07.14	ws	support for isAmpel
// 26.09.18	ws	use USBaccessDevTypes.h


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the USBACCESS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// USBACCESS_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
 

#ifndef __USBACCESSBASIC_H__
#define __USBACCESSBASIC_H__

#define CLEWARE_DEBUG 1

#define INVALID_HANDLE_VALUE -1

enum USBtype_enum {	
#include "USBaccessDevTypes.h"	
} ;

enum USBactions {		LEDs=0, EEwrite=1, EEread=2, Reset=3, KeepCalm=4, GetInfo=5, 
						StartMeasuring=6,		// USB-Humidity
						Configure=7,			// USB-IO16-V10, USB-Counter-V05
						Display=8,				// USB/Display
						RunPoint=10,			// USB-Encoder
						EEread4=13,				// 613
						Programm=15				// Transfer new Firmware (internal use only)
						} ;

// 03.11.02	ws	small changes for Linux
typedef struct {
	unsigned long int	handle ;
	int					gadgetVersionNo ;
	enum USBtype_enum	gadgettype ;
	int					SerialNumber ;
	int					report_type ;
	int					HWversion ;
	int					isAmpel ;
	} cwSUSBdata ;


void 						cwInitCleware() ;
int							cwOpenCleware() ;	// returns number of found Cleware devices
void						cwCloseCleware() ;
int							cwGetValue(int deviceNo, int UsagePage, int Usage, unsigned char *buf, int bufsize) ;
int							cwSetValue(int deviceNo, int UsagePage, int Usage, unsigned char *buf, int bufsize) ;
unsigned long int 			cwGetHandle(int deviceNo) ;
int							cwGetVersion(int deviceNo) ;
enum USBtype_enum			cwGetUSBType(int deviceNo) ;
int							cwGetSerialNumber(int deviceNo) ;
int							cwRecover(int devNum) ;		// try to find disconnected devices
int							cwValidSerNum(int SerialNumber, enum USBtype_enum devType) ;
int							cwGetHWversion(int deviceNo) ;			// return current
int							cwIsAmpel(int deviceNo) ;
int							cwIOX(int deviceNo, int addr, int data) ;
void						cwDebugWrite(char *s) ;
void						cwDebugClose() ;

#endif // __USBACCESS_H__
