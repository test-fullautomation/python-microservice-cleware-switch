// Basic class definitions for access to USB HID devices
//
/* Copyright (C) 2001-2022 Copyright Cleware GmbH, Wilfried Söker
 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
//
// History:
// 05.01.01	ws	Initial coding
// 17.07.01	ws	cleanup interface
// 12.05.06	ws	lift to Version 3.3.0
// 25.05.13	ws	support for new controller devices
// 08.07.14	ws	support for isAmpel
// 26.09.18	ws	use USBaccessDevTypes.h
// 06.10.21 ws	add internal object to avoid global var
// 14.03.23 ws	add PT100 support


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
	int				  ADCtype ;
	double			ADC_factor ;		// multiply with this is used for PT100
	double			ADC_delta ;			// add this is used for PT100
	} cwSUSBdata ;


cwSUSBdata						*cwInitCleware() ;
int							cwOpenCleware(cwSUSBdata *ud) ;	// returns number of found Cleware devices
void						cwCloseCleware(cwSUSBdata *ud) ;
int							cwGetValue(cwSUSBdata *ud, int deviceNo, int UsagePage, int Usage, unsigned char *buf, int bufsize) ;
int							cwSetValue(cwSUSBdata *ud, int deviceNo, int UsagePage, int Usage, unsigned char *buf, int bufsize) ;
unsigned long int 			cwGetHandle(cwSUSBdata *ud, int deviceNo) ;
int							cwGetVersion(cwSUSBdata *ud, int deviceNo) ;
enum USBtype_enum			cwGetUSBType(cwSUSBdata *ud, int deviceNo) ;
int							cwGetSerialNumber(cwSUSBdata *ud, int deviceNo) ;
int							cwRecover(cwSUSBdata *ud, int devNum) ;		// try to find disconnected devices
int							cwValidSerNum(int SerialNumber, enum USBtype_enum devType) ;
int							cwGetHWversion(cwSUSBdata *ud, int deviceNo) ;			// return current
int							cwIsAmpel(cwSUSBdata *ud, int deviceNo) ;
int							cwGetADCtype(cwSUSBdata *ud, int deviceNo) ;			// return the type found inside the ADC
int							cwIOX(cwSUSBdata *ud, int deviceNo, int addr, int data) ;
int							cwDecodeBCD(unsigned char *ba, int baLength, double *zahl) ;	// returns 1 if ok, 0 if failedvoid						cwDebugWrite(char *s) ;
double					cwGet_ADC_factor(cwSUSBdata *ud, int deviceNo) ;
double					cwGet_ADC_delta(cwSUSBdata *ud, int deviceNo) ;
void						cwDebugWrite(char *s) ;
void						cwDebugClose() ;

#endif // __USBACCESS_H__
