// Basic class implementation for access to USB HID devices
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
// 07.08.20 ws	reactivate Linux signbit problem fix
// 06.10.21 ws	add internal object to avoid global var
// 04.11.22 ws	Linux signbit problem fix in SetSwitch calling MultiSwitch
// 28.02.23 ws	Fix for USB-Ampel4  
// 01.03.23 ws	Fix new Humi interface, shows Error at higher temperature  

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "USBaccess.h"


CUSBaccess::CUSBaccess() {
	cwBasicObj = cwInitCleware() ;
	}

CUSBaccess::~CUSBaccess() {
	cwCloseCleware(cwBasicObj) ;		// just to be sure allocated memory got free
	cwBasicObj = 0 ;
	}


// returns number of found Cleware devices
int
CUSBaccess::OpenCleware() {
	int rval = cwOpenCleware(cwBasicObj) ;

	return rval ;
	}

int
CUSBaccess::Recover(int devNum) {
	int rval = cwRecover(cwBasicObj, devNum) ;

	return rval ;
	}

// return true if ok, else false
int
CUSBaccess::CloseCleware() {
	int rval = 1 ;
	
	cwCloseCleware(cwBasicObj) ;
	cwBasicObj = 0 ;

	return rval ;
	}

int 
CUSBaccess::GetVersion(int deviceNo) { 
	return cwGetVersion(cwBasicObj, deviceNo) ; 
	}

int 
CUSBaccess::GetUSBType(int deviceNo) { 
	int devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	int devVersion = cwGetVersion(cwBasicObj, deviceNo) ;
	if (devType == CONTACT00_DEVICE && devVersion <= 12 && devVersion > 5) {
		// this may be an early switch3/4 build on base of contact HW - adjust the ID
		int switchCount = 0 ;
		for (int autoCnt=4 ; autoCnt > 0 ; autoCnt--) {
			const int bufSize = 6 ;
			unsigned char buf[bufSize] = { 0, 0, 0, 0, 0, 0 } ;
			int seqNumber = SyncDevice(deviceNo, 0xffff) ;
			Sleep(20) ;

			for (int securityCnt=50 ; switchCount == 0 && seqNumber != 0 && securityCnt > 0 ; securityCnt--) {
				if (GetValue(deviceNo, buf, bufSize)) {
					if (buf[1] == seqNumber) {
						switchCount = buf[0] & 0x7f ;
						break ;
						}
					}
				else {
					securityCnt /= 10 ;		// don't wait too long if GetValue failed
					Sleep(20) ;
					}
				}
			}
		if (switchCount > 0 && switchCount <= 8)
			devType = SWITCHX_DEVICE ;
		}
	return devType ;
	}

int	 
CUSBaccess::GetSerialNumber(int deviceNo) { 
	return cwGetSerialNumber(cwBasicObj, deviceNo) ; 
	}



// returns 1 if ok or 0 in case of an error
int		
CUSBaccess::GetValue(int deviceNo, unsigned char *buf, int bufsize) {
	int rval = cwGetValue(cwBasicObj, deviceNo, 65441, 3, buf, bufsize) ;

	return rval ;
	}


int 
CUSBaccess::SetValue(int deviceNo, unsigned char *buf, int bufsize) {
	int rval = cwSetValue(cwBasicObj, deviceNo, 65441, 4, buf, bufsize) ;
	
	return rval ;
	}

int 
CUSBaccess::SetLED(int deviceNo, enum LED_IDs Led, int value) {
	unsigned char s[6] ;
	int rval = 0 ;
	
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	int version = cwGetVersion(cwBasicObj, deviceNo) ;

	if (devType == LED_DEVICE && version <= 10) {
		s[0] = Led ;
		s[1] = value ;
		rval = SetValue(deviceNo, s, 2) ;
		}
	else if (devType == TEMPERATURE2_DEVICE || devType == HUMIDITY1_DEVICE) {
		s[0] = 0 ;
		s[1] = Led ;
		s[2] = value ;
		s[3] = 0 ;
		rval = SetValue(deviceNo, s, 4) ;
		}
	else if (devType == ENCODER01_DEVICE) {
		s[0] = 0 ;
		s[1] = Led ;
		s[2] = value ;
		s[3] = 0 ;
		s[4] = 0 ;
		s[5] = 0 ;
		rval = SetValue(deviceNo, s, 6) ;
		}
	else if ((devType == CONTACT00_DEVICE && version > 6) || devType == KEYC01_DEVICE || devType == KEYC16_DEVICE || devType == WATCHDOGXP_DEVICE || devType == SWITCHX_DEVICE) {		// 5 bytes to send
		s[0] = 0 ;
		s[1] = Led ;
		s[2] = value ;
		s[3] = 0 ;
		s[4] = 0 ;
		rval = SetValue(deviceNo, s, 5) ;
		}
	else {
		s[0] = 0 ;
		s[1] = Led ;
		s[2] = value ;
		rval = SetValue(deviceNo, s, 3) ;
		}

	return rval ;
	}

int 
CUSBaccess::SetSwitch(int deviceNo, enum SWITCH_IDs Switch, int On) {
	unsigned char s[6] ;
	int rval = 0 ;
	int version = cwGetVersion(cwBasicObj, deviceNo) ;
	
	if (Switch < SWITCH_0 || Switch > SWITCH_15)
		return -1 ;

	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	if (devType == SWITCH1_DEVICE || devType == AUTORESET_DEVICE || devType == WATCHDOG_DEVICE) {
		s[0] = 0 ;
		s[1] = Switch ;
		if (version < 4)	// old version do not invert
			s[2] = !On ;
		else
			s[2] = On ;
		rval = SetValue(deviceNo, s, 3) ;
		if (rval && Switch == SWITCH_0) {			// set LED for first switch
			if (On) {
				SetLED(deviceNo, LED_0, 0) ;	// USB Switch will invert LED
				SetLED(deviceNo, LED_1, 15) ;
				}
			else {
				SetLED(deviceNo, LED_0, 15) ;
				SetLED(deviceNo, LED_1, 0) ;
				}
			}
		}
	else if (devType == ENCODER01_DEVICE) {
		s[0] = 0 ;
		s[1] = Switch ;
		s[2] = On ;
		s[3] = 0 ;
		s[4] = 0 ;
		s[5] = 0 ;
		rval = SetValue(deviceNo, s, 6) ;
		}
	else if (devType == CONTACT00_DEVICE && version > 6) {		// 5 bytes to send
		if (IsAmpel(deviceNo) == 4)  { // This is Ampel4
			s[0] = 0 ;
			s[1] = Switch ;
			s[2] = On ;
			rval = SetValue(deviceNo, s, 5) ;
			}
		else {
			int mask = 1 << (Switch - SWITCH_0) ;		// setup mask
			int data = 0 ;
			if (On)
				data = mask ;
//		if (GetHWversion(deviceNo) == 0)		// old IO16
				s[0] = 3 ;
//		else									// new 613 device
//			s[0] = ContactWrite ;			// in case of Linux sign bit problem, use 3 because 0xb0 set sign bit!!
	// LINUX sign bit problem
			s[0] = s[0] << 4 ;
			if (data & 0x8000)
				s[0] |= 0x08 ;
			if (data & 0x80)
				s[0] |= 0x04 ;
			if (mask & 0x8000)
				s[0] |= 0x02 ;
			if (mask & 0x80)
				s[0] |= 0x01 ;
			s[1] = (unsigned char)(data >> 8)  & 0x7f ;
			s[2] = (unsigned char)(data & 0xff) & 0x7f  ;
			s[3] = (unsigned char)(mask >> 8)  & 0x7f ;
			s[4] = (unsigned char)(mask & 0xff) & 0x7f  ;
/* end of LINUX sign bit fix		
		s[1] = data >> 8 ;
		s[2] = data & 0xff ;
		s[3] = mask >> 8 ;
		s[4] = mask & 0xff ;
*/		
			rval = SetValue(deviceNo, s, 5) ;
			}
		}
else if (devType == SWITCHX_DEVICE || devType == WATCHDOGXP_DEVICE) {		// 5 bytes to send
		int mask = 1 << (Switch - SWITCH_0) ;		// setup mask
		int data = 0 ;
		if (On)
			data = mask ;
//		if (GetHWversion(deviceNo) == 0)		// old IO16
			s[0] = 3 ;
//		else									// new 613 device
//			s[0] = ContactWrite ;			// in case of Linux sign bit problem, use 3 because 0xb0 set sign bit!!
	// LINUX sign bit problem
		s[0] = s[0] << 4 ;
		if (data & 0x8000)
			s[0] |= 0x08 ;
		if (data & 0x80)
			s[0] |= 0x04 ;
		if (mask & 0x8000)
			s[0] |= 0x02 ;
		if (mask & 0x80)
			s[0] |= 0x01 ;
		s[1] = (unsigned char)(data >> 8)  & 0x7f ;
		s[2] = (unsigned char)(data & 0xff) & 0x7f  ;
		s[3] = (unsigned char)(mask >> 8)  & 0x7f ;
		s[4] = (unsigned char)(mask & 0xff) & 0x7f  ;
/* end of LINUX sign bit fix		
		s[1] = data >> 8 ;
		s[2] = data & 0xff ;
		s[3] = mask >> 8 ;
		s[4] = mask & 0xff ;
*/		
		rval = SetValue(deviceNo, s, 5) ;
		}
	else if (devType == COUNTER00_DEVICE) {
		s[0] = 0 ;
		s[1] = Switch ;
		s[2] = On ;
		rval = SetValue(deviceNo, s, 3) ;
		}
	else
		rval = -1 ;

	return rval ;
	}

int		// 0=error, else=ok 
CUSBaccess::GetSwitchConfig(int deviceNo, int *switchCount, int *buttonAvailable) {
	const int bufSize = 6 ;
	unsigned char buf[bufSize] = { 0, 0, 0, 0, 0, 0 } ;
	int ok = 0 ;
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	int version = cwGetVersion(cwBasicObj, deviceNo) ;

	if ((devType == CONTACT00_DEVICE && version >= 5) || devType == SWITCHX_DEVICE || devType == WATCHDOGXP_DEVICE) {
		*switchCount = 0 ;
		for (int autoCnt=4 ; autoCnt > 0 ; autoCnt--) {
			int seqNumber = SyncDevice(deviceNo, 0xffff) ;
			Sleep(20) ;

			for (int securityCnt=50 ; seqNumber != 0 && securityCnt > 0 ; securityCnt--) {
				if (GetValue(deviceNo, buf, bufSize)) {
					if (buf[1] == seqNumber) {
						ok = 1 ;
						*switchCount = buf[0] & 0x7f ;
						break ;
						}
					}
				else {
					securityCnt /= 10 ;		// don't wait too long if GetValue failed
					Sleep(20) ;
					}
				}
			if (ok >= 0)
				break ;
			}
		if (buttonAvailable)
			*buttonAvailable = 0 ;
		return ok ;
		}

	if (devType == COUNTER00_DEVICE) {
		*switchCount = 2 ;
		if (buttonAvailable)
			*buttonAvailable = 0 ;
		return ok ;
		}

	if (	devType == SWITCH1_DEVICE 
		 || devType == AUTORESET_DEVICE 
		 || devType == WATCHDOG_DEVICE 
		 || devType == F4_DEVICE) {
		*switchCount = 1 ;
		*buttonAvailable = 0 ;
		if (version >= 10) {	
			ok = (GetValue(deviceNo, buf, bufSize) && (buf[0] & 0x80) ) ;
			if (ok) {
				*switchCount = 1 ;
				if (buf[0] & 0x02)
					*switchCount = 2 ;
				if (buf[0] & 0x08)
					*switchCount = 3 ;
				if (buf[0] & 0x20) {
					if (*switchCount == 3)
						*switchCount = 4 ;				// only single switches may have a start button
					else
						*buttonAvailable = 1 ;
					}
				}
			}
		else
			ok = 1 ;
		}

	return ok ;
	}

int		// On 0=off, 1=on, -1=error	 
CUSBaccess::GetSwitch(int deviceNo, enum SWITCH_IDs Switch) {
	const int bufSize = 6 ;
	unsigned char buf[bufSize] ;
	int ok = 0 ;
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;

	if (		devType != SWITCH1_DEVICE 
			 && devType != AUTORESET_DEVICE 
			 && devType != WATCHDOG_DEVICE 
			 && devType != WATCHDOGXP_DEVICE 
			 && devType != F4_DEVICE 
			 && devType != SWITCHX_DEVICE 
			 && devType != CONTACT00_DEVICE 
			 && devType != COUNTER00_DEVICE 
			 && devType != ENCODER01_DEVICE)
		return -1 ;

	if (Switch < SWITCH_0 || Switch > SWITCH_15)
		return -1 ;

	int version = cwGetVersion(cwBasicObj, deviceNo) ;

	if ((devType == CONTACT00_DEVICE && version > 6) || devType == SWITCHX_DEVICE || devType == WATCHDOGXP_DEVICE) {		// 5 bytes to send
		unsigned long int mask = 1 << (Switch - SWITCH_0) ;		// setup mask
		unsigned long int data = 0 ;
		ok = GetMultiSwitch(deviceNo, &mask, &data, 0) ;	// mask is change ,ask on return
		mask = 1 << (Switch - SWITCH_0) ;		// setup mask
		if (ok >= 0)
			ok = (data & mask) ? 1 : 0 ;
		}

	else if (1 || version < 10) {					// else only if in separate thread
		if (GetValue(deviceNo, buf, bufSize)) {
			int mask = 1 << ((Switch - SWITCH_0) * 2) ;
			if (version >= 10 || devType == CONTACT00_DEVICE || devType == COUNTER00_DEVICE || devType == F4_DEVICE)
				ok = (buf[0] & mask) ? 1 : 0 ;
			else	// old switch
				ok = (buf[2] & mask) ? 1 : 0 ;
			}
		else
			ok = -1 ;	// getvalue failed - may be disconnected

		if (ok >= 0 && version < 4 && devType != CONTACT00_DEVICE && devType != COUNTER00_DEVICE&& devType != F4_DEVICE)
			ok = !ok ;
		}
	else {		// new version - ask for online count to get a fast answer (use this only if in separate thread)
		static int sequenceNumber = 1 ;

		buf[0] = GetInfo ;
		buf[1] = OnlineCount ;
		buf[2] = sequenceNumber ;
		SetValue(deviceNo, buf, 3) ;
		for (int timeout=25 ; timeout > 0 ; timeout--) {
			Sleep(25) ;
			if (GetValue(deviceNo, buf, bufSize)) {
				if ((buf[0] & 0x80) == 0)	// valid bit
					continue ;
				if (buf[1] != ( (sequenceNumber & 0x1f) << 3 ) + OnlineCount)
					continue ;
				ok = buf[0] & 1 ;
				break ;
				}
			}

		++sequenceNumber ;
		sequenceNumber &= 0x1f ;
		}

	return ok ;
	}

int		// On 0=off, 1=on, -1=error	 ; the seqNum is generated by the Start command.
CUSBaccess::GetSeqSwitch(int deviceNo, enum SWITCH_IDs Switch, int seqNumber) {
	const int bufSize = 6 ;
	unsigned char buf[bufSize] ;
	int ok = 0 ;
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;

	if (		devType != SWITCH1_DEVICE 
			 && devType != AUTORESET_DEVICE 
			 && devType != WATCHDOG_DEVICE 
			 && devType != F4_DEVICE 
			 && devType != CONTACT00_DEVICE 
			 && devType != SWITCHX_DEVICE 
			 && devType != COUNTER00_DEVICE 
			 && devType != ENCODER01_DEVICE)
		return -1 ;

	if (Switch < SWITCH_0 || Switch > SWITCH_15)
		return -1 ;

	int version = cwGetVersion(cwBasicObj, deviceNo) ;
	if (version < 20 && devType != CONTACT00_DEVICE && devType != SWITCHX_DEVICE && devType != COUNTER00_DEVICE && devType != F4_DEVICE)
		return -1 ;

	if (seqNumber == 0)			// do this internally
		seqNumber = StartDevice(deviceNo) ;

	buf[1] = 0 ;
	for (int securityCnt=20 ; buf[1] != seqNumber && securityCnt > 0 ; securityCnt--) {
		if (GetValue(deviceNo, buf, bufSize)) {
			int mask = 1 << ((Switch - SWITCH_0) * 2) ;
			ok = (buf[0] & mask) ? 1 : 0 ;
			}
		else {
			ok = -1 ;	// getvalue failed - may be disconnected
			break ;
			}
		}

	return ok ;
	}

int		// rval seqNum = ok, -1 = error	 
CUSBaccess::GetMultiSwitch(int deviceNo, unsigned long int *mask, unsigned long int *value, int seqNumber) {
	unsigned char buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 } ;
	int bufSize ;
	int ok = -1 ;
	int automatic = 0 ;

	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	int version = cwGetVersion(cwBasicObj, deviceNo) ;

	if (devType == SWITCH1_DEVICE) {
		int rval = 0 ;
		ok = GetSwitch(deviceNo, SWITCH_0) ;
		if (ok >= 0) {
			rval = ok ;
			ok = GetSwitch(deviceNo, SWITCH_1) ;
			}
		if (ok >= 0) {
			rval |= (ok << 1) ;
			ok = GetSwitch(deviceNo, SWITCH_2) ;
			}
		if (ok >= 0) {
			rval |= (ok << 2) ;
			ok = GetSwitch(deviceNo, SWITCH_3) ;
			}
		if (ok >= 0) {
			*value = rval | (ok << 3) ;
			ok = seqNumber ;
			}

		return ok ;
		}

	if (devType == KEYC16_DEVICE) 
		bufSize = 8 ;
	else if (devType == CONTACT00_DEVICE || devType == KEYC01_DEVICE || devType == SWITCHX_DEVICE || devType == WATCHDOGXP_DEVICE) {
		if (version < 5)
			return -1 ;
		bufSize = 6 ;
		}
	else if (devType == MOUSE_DEVICE) {
		bufSize = 4 ;
		}
	else
		return -1 ;

	if (value == 0)
		return -1 ;

	if (seqNumber == 0)			// do this internally
		automatic = 1 ;

	int readMask = 0 ;
	if (mask)
		readMask = *mask ;
	if (readMask == 0)
		readMask = 0xffff ;		// get every single bit!!

	for (int autoCnt=4 ; autoCnt > 0 ; autoCnt--) {
		if (automatic) {
			seqNumber = SyncDevice(deviceNo, readMask) ;
			Sleep(20) ;
			}

		for (int securityCnt=50 ; seqNumber != 0 && securityCnt > 0 ; securityCnt--) {
			if (GetValue(deviceNo, buf, bufSize)) {
				if ( (buf[0] & 0x80) == 0)		// this bit indicate valid IO data
					continue ;
				if (mask != 0 && !IsIdeTec(deviceNo))
					*mask =  (buf[2] << 8) + buf[3] ;
				unsigned long int v = (buf[4] << 8) + buf[5] ;
				if (version < 7 && devType != KEYC16_DEVICE && devType != KEYC01_DEVICE)
					*value = 0xffff & ~v ;
				else
					*value = v ;
				if (buf[1] == seqNumber) {
					ok = seqNumber ;
					break ;
					}
			//	Sleep(50) ;				don't sleep - we just killing the USB fifo
				}
			else {
				securityCnt /= 10 ;		// don't wait too long if GetValue failed
				Sleep(20) ;
				}
			}
		if (ok >= 0 || automatic == 0)
			break ;
		}

	return ok ;
	}

int		// On 0=ok, -1=error	 
CUSBaccess::SetMultiSwitch(int deviceNo, unsigned long int value) {
	const int bufSize = 5 ;
	unsigned char buf[bufSize] ;
	int ok = -1 ;
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;

	if (devType == SWITCH1_DEVICE) {
		ok = SetSwitch(deviceNo, SWITCH_0, value & 1) ;
		if (ok)
			ok = SetSwitch(deviceNo, SWITCH_1, value & 2) ;
		if (ok)
			ok = SetSwitch(deviceNo, SWITCH_2, value & 4) ;
		if (ok)
			ok = SetSwitch(deviceNo, SWITCH_3, value & 8) ;

		return ok ;
		}

	if (devType != CONTACT00_DEVICE && devType != SWITCHX_DEVICE && devType != WATCHDOGXP_DEVICE)
		return -1 ;

	int version = cwGetVersion(cwBasicObj, deviceNo) ;
	if (version < 5)
		return -1 ;

	if (devType == CONTACT00_DEVICE && version >= 32 && version < 48)
		return -1 ;			// this device misses 9555

//	if (GetHWversion(deviceNo) == 0)		// old IO16
		buf[0] = 3 ;
//	else									// new 613 device
	//	buf[0] = ContactWrite ;				// in case of Linux sign bit problem, use 3 because 0xb0 set sign bit!!
// LINUX sign bit problem
	buf[0] = buf[0] << 4 ;
	if (value & 0x8000)
		buf[0] |= 0x08 ;
	if (value & 0x80)
		buf[0] |= 0x04 ;
	buf[0] |= 3 ;	// mask bits
	buf[1] = (unsigned char)(value >> 8)  & 0x7f ;
	buf[2] = (unsigned char)(value & 0xff) & 0x7f  ;
	buf[3] = 0x7f ;
	buf[4] = 0x7f ;
/* end of sign bit fix
	buf[1] = (unsigned char)(value >> 8) ;
	buf[2] = (unsigned char)(value & 0xff) ;
	buf[3] = 0xff ;
	buf[4] = 0xff ;
*/
	if (SetValue(deviceNo, buf, version > 6 ? 5 : 3))
		ok = 0 ;

	return ok ;
	}

int		// On 0=ok, -1=error	 
CUSBaccess::SetMultiConfig(int deviceNo, unsigned long int directions) {	// 1=input, 0=output
	const int bufSize = 5 ;
	unsigned char buf[bufSize] ;
	int ok = -1 ;
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;

	if (devType == SWITCH1_DEVICE)
		return 0 ;		// don't care

	if (devType != CONTACT00_DEVICE && devType != SWITCHX_DEVICE && devType != WATCHDOGXP_DEVICE) 
		return -1 ;

	int version = cwGetVersion(cwBasicObj, deviceNo) ;
	if (version < 5)
		return -1 ;

/* orginal
	if (version < 10)
		buf[0] = KeepCalm ;			// dirty old code
	else
		buf[0] = Configure ;
	buf[1] = (unsigned char)(directions >> 8) ;
	buf[2] = (unsigned char)(directions & 0xff) ;
	buf[3] = 0 ;
	buf[4] = 0 ;
*/
// LINUX sign bit problem
	if (version < 10)
		buf[0] = 4 << 4 ;		// dirty old code
	else
		buf[0] = 7 << 4 ;
	if (directions & 0x8000)
		buf[0] |= 0x08 ;
	if (directions & 0x80)
		buf[0] |= 0x04 ;
	buf[1] = (unsigned char)(directions >> 8)  & 0x7f ;
	buf[2] = (unsigned char)(directions & 0xff) & 0x7f  ;
	buf[3] = 0 ;
	buf[4] = 0 ;

	if (SetValue(deviceNo, buf, version > 6 ? 5 : 3))
		ok = 0 ;

	return ok ;
	}


int		// -1=error	 otherwise bits 0 - 15 	1=input, 0=output
CUSBaccess::GetMultiConfig(int deviceNo) {
	const int bufSize = 6 ;
	unsigned char buf[bufSize] ;
	int rval = -1 ;
// 	deviceNo = X->SerNum2DeviceNo(deviceNo);

	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;

	if (devType != CONTACT00_DEVICE && devType != SWITCHX_DEVICE && devType != WATCHDOGXP_DEVICE) 
		return -1 ;

	int version = cwGetVersion(cwBasicObj, deviceNo) ;
	if (version < 5)
		return -1 ;

	int lowbyte = IOX(deviceNo, 7, -1) ;
	int highbyte = IOX(deviceNo, 6, -1) ;

	rval = (highbyte << 8) + lowbyte ;

	return rval ;
	}

int		// // return value of counter (0 or 1 for USB-IO16) or -1 in case of an error
CUSBaccess::GetCounter(int deviceNo, enum COUNTER_IDs counterID) {
	const int bufSize = 6 ;
	unsigned char buf[bufSize] ;
	int rval = -1 ;
	static int sequenceNumber = 1 ;
	int sendlen = bufSize ;
	int isIO16 = false ;

	++sequenceNumber ;
	sequenceNumber &= 0x1f ;

	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	int version = cwGetVersion(cwBasicObj, deviceNo) ;

	if (devType == CONTACT00_DEVICE) {
		sendlen = 5 ;
		if (version >= 6)
			isIO16 = true ;
		}
	else if (devType == COUNTER00_DEVICE) 
		sendlen = 3 ;
	else if (devType == CONTACTTIMER00_DEVICE)
		sendlen = 5 ;
	else
		return -1 ;

	for (int autoCnt=4 ; autoCnt > 0 ; autoCnt--) {
		buf[0] = CUSBaccess::GetInfo ;
		buf[1] = sequenceNumber ;
		buf[2] = 0 ;
		buf[3] = 0 ;
		buf[4] = 0 ;

		if (!SetValue(deviceNo, buf, sendlen)) {
			Sleep(50) ;
			continue ;
			}
		Sleep(20) ;

		buf[1] = 0 ;
		for (int securityCnt=50 ; securityCnt > 0 ; securityCnt--) {
			if (GetValue(deviceNo, buf, bufSize)) {
				if (isIO16 && buf[0] != 0xff) {			// 0xff indicates that the counters are prepared 
					Sleep(10) ;
					continue ;
					}
				if (buf[1] != sequenceNumber) {
					Sleep(10) ;
					continue ;
					}
				if (!isIO16)
					rval = (buf[2] << 24) + (buf[3] << 16) + (buf[4] << 8) + buf[5] ;
				else {
					if (counterID == 0)
						rval = (buf[2] << 8) + buf[3] ;
					else
						rval = (buf[4] << 8) + buf[5] ;
					}
				break ;
				//	Sleep(50) ;				don't sleep - we just killing the USB fifo
				}
			else {
				if (securityCnt > 10)
					securityCnt /= 10 ;		// don't wait too long if GetValue failed
				Sleep(20) ;
				}
			}
		if (rval >= 0)
			break ;
		}

	return rval ;
	}

int		// // return value of counter (0 or 1 for USB-IO16) or -1 in case of an error
CUSBaccess::GetFrequency(int deviceNo, unsigned long int *counter, int subDevice) {
	const int bufSize = 6 ;
	unsigned char buf[bufSize] ;
	int rval = -1 ;

	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	int version = cwGetVersion(cwBasicObj, deviceNo) ;

	if (devType != COUNTER00_DEVICE || version < 0x101) 
		return -1 ;

	for (int securityCnt=5 ; securityCnt > 0 ; securityCnt--) {
		if (GetValue(deviceNo, buf, bufSize)) {
			if ( (buf[0] & 0xc0) != 0xc0) {			// 0xc0 indicate frequency is valid
				Sleep(10) ;
				continue ;
				}
			int sub = (buf[0] >> 4) & 0x03 ;
			if (sub != subDevice) {			// wrong channel
				Sleep(10) ;
				continue ;
				}
			*counter = ((buf[0] & 0x0f) << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3] ;
			rval = (buf[4] << 8) + buf[5] ;
			break ;
			}
		else {
			Sleep(20) ;
			}
		}

	return rval ;
	}


int
CUSBaccess::SetCounter(int deviceNo, int counter, enum COUNTER_IDs counterID) {	//  -1=error, COUNTER_IDs ununsed until now
	const int bufSize = 3 ;
	unsigned char buf[bufSize] ;
	int ok = -1 ;

	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	int version = cwGetVersion(cwBasicObj, deviceNo) ;

	if (devType == COUNTER00_DEVICE) {
		buf[0] = CUSBaccess::Configure ;
		buf[1] = counter >> 8 ;
		buf[2] = counter & 0xff ;
		if (SetValue(deviceNo, buf, bufSize))
			ok = 0 ;
		}

	if (devType == COUNTER00_DEVICE) {
		if (version == 0x101 || (version > 0x101 && counter > 0xffff)) {
			int cnt = (counter >> 24) & 0xff ;
			IOX(deviceNo, 16, cnt) ;
			cnt = (counter >> 16) & 0xff ;
			IOX(deviceNo, 17, cnt) ;
			cnt = (counter >>  8) & 0xff ;
			IOX(deviceNo, 18, cnt) ;
			cnt = counter & 0xff ;
			ok = IOX(deviceNo, 19, cnt) ;
			}
		else if (version > 0x101) {		// ActionConfigDevice no supported
			buf[0] = CUSBaccess::Configure ;
			buf[1] = (counter >> 8) & 0xff ;
			buf[2] = (counter     ) & 0xff ;
			if (SetValue(deviceNo, buf, bufSize))
				ok = 0 ; 
			}
		else {
			buf[0] = CUSBaccess::Configure ;
			buf[1] = counter >> 8 ;
			buf[2] = counter & 0xff ;
			if (SetValue(deviceNo, buf, bufSize))
				ok = 0 ; 
			}
		}

	return ok ;
	}


int		// returns how often switch is manually turned on, -1 in case of an error
CUSBaccess::GetManualOnCount(int deviceNo) {
	const int bufSize = 6 ;
	unsigned char buf[bufSize] ;
	int rval = -1 ;
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	static int sequenceNumber = 1 ;

	if (	(	devType == SWITCH1_DEVICE 
			 || devType == AUTORESET_DEVICE 
			 || devType == CONTACT00_DEVICE 
			 || devType == WATCHDOG_DEVICE)
			&& cwGetVersion(cwBasicObj, deviceNo) >= 10) {
		for (int timeout=5 ; timeout > 0 ; timeout--) {
			buf[0] = GetInfo ;
			buf[1] = ManualCount ;
			buf[2] = sequenceNumber ;
			if (devType == CONTACT00_DEVICE)
				SetValue(deviceNo, buf, 5) ;
			else
				SetValue(deviceNo, buf, 3) ;
			for (int timeout2=3 ; timeout2 > 0 ; timeout2--) {
				Sleep(50) ;
				if (GetValue(deviceNo, buf, bufSize)) {
					if ((buf[0] & 0x80) == 0)	// valid bit
						continue ;
					if (buf[1] != ( (sequenceNumber & 0x1f) << 3 ) + ManualCount)
						continue ;
					if ((buf[5] & 0x80) == 0)	// valid data bit
						continue ;
					rval = buf[2] + (buf[3] << 8) + (buf[4] << 16) + ((buf[5] & 0x7f) << 24) ;
					break ;
					}
				}
			if (rval != -1)
				break ;
			Sleep(250) ;
			}
		}

	++sequenceNumber ;
	sequenceNumber &= 0x1f ;

	return rval ;
	}

int		// returns how long (seconds) switch is manually turned on, -1 in case of an error
CUSBaccess::GetManualOnTime(int deviceNo) {
	const int bufSize = 6 ;
	unsigned char buf[bufSize] ;
	int rval = -1 ;
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	static int sequenceNumber = 1 ;

	if (	(	devType == SWITCH1_DEVICE 
			 || devType == AUTORESET_DEVICE 
			 || devType == CONTACT00_DEVICE 
			 || devType == WATCHDOG_DEVICE)
			&& cwGetVersion(cwBasicObj, deviceNo) >= 10) {
		for (int timeout=5 ; timeout > 0 ; timeout--) {
			buf[0] = GetInfo ;
			buf[1] = ManualTime ;
			buf[2] = sequenceNumber ;
			if (devType == CONTACT00_DEVICE)
				SetValue(deviceNo, buf, 5) ;
			else
				SetValue(deviceNo, buf, 3) ;
			for (int timeout2=3 ; timeout2 > 0 ; timeout2--) {
				Sleep(50) ;
				if (GetValue(deviceNo, buf, bufSize)) {
					if ((buf[0] & 0x80) == 0)	// valid bit
						continue ;
					if (buf[1] != ( (sequenceNumber & 0x1f) << 3 ) + ManualTime)
						continue ;
					if ((buf[5] & 0x80) == 0)	// valid data bit
						continue ;
					rval = buf[2] + (buf[3] << 8) + (buf[4] << 16) + ((buf[5] & 0x7f) << 24) ;
					break ;
					}
				}
			if (rval != -1)
				break ;
			Sleep(250) ;
			}
		}

	if (rval >= 0 && devType != CONTACT00_DEVICE) {	// rval is 256 * 1,024 ms
		double u_seconds = 256. * 1024. ;
		u_seconds *= rval ;
		rval = (int) (u_seconds / 1000000) ;
		}

	++sequenceNumber ;
	sequenceNumber &= 0x1f ;

	return rval ;
	}

int		// returns how often switch is turned on by USB command, -1 in case of an error
CUSBaccess::GetOnlineOnCount(int deviceNo) {
	const int bufSize = 6 ;
	unsigned char buf[bufSize] ;
	int rval = -1 ;
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	static int sequenceNumber = 1 ;
	int timeout=-1, timeout2=-1 ;

	if (	(	devType == SWITCH1_DEVICE 
			 || devType == AUTORESET_DEVICE 
			 || devType == CONTACT00_DEVICE 
			 || devType == WATCHDOGXP_DEVICE 
			 || devType == WATCHDOG_DEVICE)
			&& cwGetVersion(cwBasicObj, deviceNo) >= 10) {
		for (timeout=5 ; timeout > 0 ; timeout--) {
			buf[0] = GetInfo ;
			if (devType == WATCHDOGXP_DEVICE) {
				buf[1] = sequenceNumber ;
				SetValue(deviceNo, buf, 5) ;
				}
			else if (devType == CONTACT00_DEVICE) {
				buf[1] = OnlineCount ;
				buf[2] = sequenceNumber ;
				SetValue(deviceNo, buf, 5) ;
				}
			else {
				buf[1] = OnlineCount ;
				buf[2] = sequenceNumber ;
				SetValue(deviceNo, buf, 3) ;
				}
			for (timeout2=3 ; timeout2 > 0 ; timeout2--) {
				Sleep(50) ;
				if (GetValue(deviceNo, buf, bufSize)) {
					if ((buf[0] & 0x80) == 0)	// valid bit
						continue ;
					if (devType == WATCHDOGXP_DEVICE) {
						if (buf[1] != sequenceNumber)
							continue ;
						}
					else {
						if (buf[1] != ( (sequenceNumber & 0x1f) << 3 ) + OnlineCount)
							continue ;
						}
					if ((buf[5] & 0x80) == 0)	// valid data bit
						continue ;
					rval = buf[2] + (buf[3] << 8) + (buf[4] << 16) + ((buf[5] & 0x7f) << 24) ;
					break ;
					}
				}
			if (rval != -1)
				break ;
			Sleep(250) ;
			}
		}

/*	static char ds[256] ;
	sprintf(ds, "GetOnlineOnCount(%d) %s, seq=%d, time1=%d, time2=%d\n", 
				deviceNo, (rval==-1)?"failed":"ok", sequenceNumber, timeout, timeout2) ;
	cwDebugWrite(ds) ;
*/
	++sequenceNumber ;
	sequenceNumber &= 0x1f ;

	return rval ;
	}

int		// returns how long (seconds) switch is turned on by USB command, -1 in case of an error
CUSBaccess::GetOnlineOnTime(int deviceNo) {
	const int bufSize = 6 ;
	unsigned char buf[bufSize] ;
	int rval = -1 ;
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	static int sequenceNumber = 1 ;

	if (	(	devType == SWITCH1_DEVICE 
			 || devType == AUTORESET_DEVICE 
			 || devType == CONTACT00_DEVICE 
			 || devType == WATCHDOG_DEVICE)
			&& cwGetVersion(cwBasicObj, deviceNo) >= 10) {
		for (int timeout=5 ; timeout > 0 ; timeout--) {
			buf[0] = GetInfo ;
			buf[1] = OnlineTime ;
			buf[2] = sequenceNumber ;
			if (devType == CONTACT00_DEVICE)
				SetValue(deviceNo, buf, 5) ;
			else
				SetValue(deviceNo, buf, 3) ;
			for (int timeout2=3 ; timeout2 > 0 ; timeout2--) {
				Sleep(50) ;
				if (GetValue(deviceNo, buf, bufSize)) {
					if ((buf[0] & 0x80) == 0)	// valid bit
						continue ;
					if (buf[1] != ( (sequenceNumber & 0x1f) << 3 ) + OnlineTime)
						continue ;
					if ((buf[5] & 0x80) == 0)	// valid data bit
						continue ;
					rval = buf[2] + (buf[3] << 8) + (buf[4] << 16) + ((buf[5] & 0x7f) << 24) ;
					break ;
					}
				}
			if (rval != -1)
				break ;
			Sleep(250) ;
			}
		}

	if (rval >= 0 && devType != CONTACT00_DEVICE) {	// rval is 256 * 1,024 ms
		double u_seconds = 256. * 1024. ;
		u_seconds *= rval ;
		rval = (int) (u_seconds / 1000000) ;
		}

	++sequenceNumber ;
	sequenceNumber &= 0x1f ;

	return rval ;
	}

int 
CUSBaccess::ResetDevice(int deviceNo) {
	int ok = 1 ;
	const int bufsize = 6 ;
	unsigned char buf[bufsize] ;
	int version = cwGetVersion(cwBasicObj, deviceNo) ;

	buf[0] = CUSBaccess::Reset ;
	buf[1] = 0 ;
	buf[2] = 0 ;
	buf[3] = 0 ;
	buf[4] = 0 ;
	buf[5] = 0 ;
	int type = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	if (type == TEMPERATURE2_DEVICE || type == HUMIDITY1_DEVICE)
		ok = SetValue(deviceNo, buf, 4) ;
	else if ((type == CONTACT00_DEVICE && version > 6) || type == SWITCHX_DEVICE || type == WATCHDOGXP_DEVICE || type == KEYC01_DEVICE || type == KEYC16_DEVICE)
		ok = SetValue(deviceNo, buf, 5) ;
	else if (type == ENCODER01_DEVICE)
		ok = SetValue(deviceNo, buf, bufsize) ;
	else
		ok = SetValue(deviceNo, buf, 3) ;

	return ok ;
	}

int 
CUSBaccess::StartDevice(int deviceNo) {		// mask in case of CONTACT00-device
	int ok = 1 ;
	const int bufsize = 5 ;
	unsigned char buf[bufsize] ;
	static int sequenceNumber = 1 ;

/* orginal
	sequenceNumber = (++sequenceNumber) & 0xff ;
	if (sequenceNumber == 0)
		sequenceNumber = 1 ;
*/
	if (++sequenceNumber > 0x7f)	// LINUX sign bit problem
		sequenceNumber = 1 ;

	buf[0] = CUSBaccess::StartMeasuring ;
	buf[1] = sequenceNumber ;
	buf[2] = 0 ;
	buf[3] = 0 ;
	buf[4] = 0 ;

	int type = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	int version = cwGetVersion(cwBasicObj, deviceNo) ;

	if (type == TEMPERATURE2_DEVICE || type == HUMIDITY1_DEVICE)
		ok = SetValue(deviceNo, buf, 4) ;
	else if ((type == CONTACT00_DEVICE && version > 6) || type == SWITCHX_DEVICE || type == WATCHDOGXP_DEVICE || type == KEYC01_DEVICE || type == KEYC16_DEVICE)
		ok = SetValue(deviceNo, buf, 5) ;
	else
		ok = SetValue(deviceNo, buf, 3) ;

	return (ok ? sequenceNumber : 0) ;
	}

int 
CUSBaccess::SyncDevice(int deviceNo, unsigned long int mask) {		// mask in case of CONTACT00-device
	int ok = 1 ;
	const int bufsize = 5 ;
	unsigned char buf[bufsize] ;
	static int sequenceNumber = 1 ;

/* orginal
	sequenceNumber = (++sequenceNumber) & 0xff ;
	if (sequenceNumber == 0)
		sequenceNumber = 1 ;
*/
	if (++sequenceNumber > 0x7f)	// LINUX signed byte
		sequenceNumber = 1 ;

	if (mask == 0)
		mask = 0xffff ;		// get every single bit!!

/* orginal
	buf[0] = CUSBaccess::StartMeasuring ;
	buf[1] = sequenceNumber ;
	buf[2] = 0 ;
	buf[3] = (unsigned char)(mask >> 8) ;
	buf[4] = (unsigned char)(mask & 0xff) ;
*/
// LINUX sign bit problem
	buf[0] = CUSBaccess::StartMeasuring << 4 ;
	if (mask & 0x8000)
		buf[0] |= 0x02 ;
	if (mask & 0x80)
		buf[0] |= 0x01 ;
	// buf[0] = CUSBaccess::StartMeasuring ;
	buf[1] = sequenceNumber ;
		buf[2] = 0 ;
		buf[3] = (unsigned char)(mask >> 8) & 0x7f ;
		buf[4] = (unsigned char)(mask & 0xff) & 0x7f ;

	int type = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;
	int version = cwGetVersion(cwBasicObj, deviceNo) ;

	if ((type == CONTACT00_DEVICE&& version > 6) || type == SWITCHX_DEVICE || type == WATCHDOGXP_DEVICE || type == KEYC01_DEVICE || type == KEYC16_DEVICE)
		ok = SetValue(deviceNo, buf, 5) ;

	return (ok ? sequenceNumber : 0) ;
	}

int 
CUSBaccess::CalmWatchdog(int deviceNo, int minutes, int minutes2restart) {
	int ok = 0 ;
	const int bufsize = 5 ;
	unsigned char buf[bufsize] ;
	USBtype_enum devType = (USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo) ;

	buf[0] = CUSBaccess::KeepCalm ;
	buf[1] = minutes ;
	buf[2] = minutes2restart ;
	if (devType == AUTORESET_DEVICE || devType == WATCHDOG_DEVICE || devType == SWITCH1_DEVICE)
		ok = SetValue(deviceNo, buf, 3) ;
	else if (devType == CONTACT00_DEVICE || devType == SWITCHX_DEVICE || devType == WATCHDOGXP_DEVICE)
		ok = SetValue(deviceNo, buf, bufsize) ;

	return ok ;
	}

static int ResetDone=0 ;
float 
CUSBaccess::GetTemperature(int deviceNo) {
	double temperatur = -200. ;
	static int zeit = 1234 ;
	int neuzeit = -1 ;
	static time_t lasttime = 0 ;
	time_t now ;
	time(&now) ;

	int devType = GetUSBType(deviceNo) ;
	int r2 ;
	for (r2 = 5 ; r2 > 0 ; r2--) {
		int retry ;
		for (retry = 20 ; retry > 0 ; retry--) {
			if (!GetTemperature(deviceNo, &temperatur, &zeit)) {
				if (ResetDone == 0)
					break ;			// waiting for first RESET, dont wait
				Sleep(200) ;		// wait a bit to settle after reset
				}
			else {
				if (zeit == neuzeit && now != lasttime) {
					ResetDone = 0 ;				// zeit didn't cjange - do reset
					neuzeit = zeit ;
					break ;
					}
				ResetDone = 1 ;					// we got a valid Temperature and the timig is ok! No Reset neccessary
				break ;
				}
			}
		if (ResetDone == 0 || retry == 0) {
			ResetDevice(deviceNo) ;
			ResetDone = 1 ;
			Sleep(700) ;		// wait a bit to settle after reset
			if (devType == HUMIDITY1_DEVICE || devType == HUMIDITY2_DEVICE) {		// Start command is needed
				StartDevice(deviceNo) ;
				Sleep(1200) ;			// takes some time to get the first valid values
				}
			continue ;
			}
		break ;
		}
	float ftemp = -200. ;
	if (r2 > 0)
		ftemp = temperatur ;
	return ftemp ;
	}

float 
CUSBaccess::GetHumidity(int deviceNo) {
	double humidity = -200. ;
	static int zeit = 1234 ;
	int neuzeit = -1 ;
	static time_t lasttime = 0 ;
	time_t now ;
	time(&now) ;
	int r2 ;

	int devType = GetUSBType(deviceNo) ;
	for (r2 = 5 ; r2 > 0 ; r2--) {
		int retry ;
		for (retry = 20 ; retry > 0 ; retry--) {
			if (!GetHumidity(deviceNo, &humidity, &zeit)) {
				if (ResetDone == 0)
					break ;			// waiting for first RESET, dont wait
				Sleep(200) ;		// wait a bit to settle after reset
				}
			else {
				if (zeit == neuzeit && now != lasttime) {
					ResetDone = 0 ;				// zeit didn't cjange - do reset
					neuzeit = zeit ;
					break ;
					}
				ResetDone = 1 ;					// we got a valid Humidity and the timig is ok! No Reset neccessary
				break ;
				}
			}
		if (ResetDone == 0 || retry == 0) {
			ResetDevice(deviceNo) ;
			ResetDone = 1 ;
			Sleep(700) ;		// wait a bit to settle after reset
			if (devType == HUMIDITY1_DEVICE || devType == HUMIDITY2_DEVICE) {		// Start command is needed
				StartDevice(deviceNo) ;
				Sleep(1200) ;			// takes some time to get the first valid values
				}
			continue ;
			}
		break ;
		}
	float ftemp = -200. ;
	if (r2 > 0)
		ftemp = humidity ;
	return ftemp ;
	}

int 
CUSBaccess::GetTemperature(int deviceNo, double *Temperature, int *timeID) {
	int ok = 1 ;
	const int maxDevs = 128 ;
	static double lastTemperature[maxDevs] ;
	static int initialized = 0 ;

	if (!initialized) {
		for (int i=0 ; i < maxDevs ; i++)
			lastTemperature[i] = -200. ;
		initialized = 1 ;
		}

	switch ((USBtype_enum)cwGetUSBType(cwBasicObj, deviceNo)) {
		case ADC0800_DEVICE: {
			static int fakeTime = 1 ;
			int subDevice = 0 ;
			float t = GetADC(deviceNo, 0, subDevice) ;
			double T = t * cwGet_ADC_factor(cwBasicObj, deviceNo) + cwGet_ADC_delta(cwBasicObj, deviceNo) ;
			*Temperature = T ;

			*timeID = fakeTime++ ;
			break ;
			}
		case TEMPERATURE_DEVICE: {
			const int bufSize = 6 ;
			unsigned char buf[bufSize] ;
			// read temperature 
			if (GetValue(deviceNo, buf, bufSize) == 0) {
				ok = 0 ;
				break ;
				}
			*timeID  = ((buf[0] & 0x7f) << 8) + buf[1] ;
			int value = (buf[2] << 5) + (buf[3] >> 3) ;
			if (value & 0x1000)		// negativ!
				value = (value & 0xfff) - 0x1000 ;
			int valid = (buf[0] & 0x80) ;	// MSB = valid-bit
			if (!valid) { // invalid time
				ok = 0 ;
				break ;
				}
			*Temperature = value * 0.0625 ;
			break ;
			}
		case TEMPERATURE2_DEVICE: {
			const int bufSize = 7 ;
			unsigned char buf[bufSize] ;
			// read temperature 
			if (GetValue(deviceNo, buf, bufSize) == 0) {
				ok = 0 ;
				break ;
				}
			*timeID  = ((buf[0] & 0x7f) << 8) + buf[1] ;
			int value = (buf[2] << 5) + (buf[3] >> 3) ;
			if (value & 0x1000)		// negativ!
				value = (value & 0xfff) - 0x1000 ;
			int valid = (buf[0] & 0x80) ;	// MSB = valid-bit
			if (!valid) { // invalid time
				ok = 0 ;
				break ;
				}
			*Temperature = value * 0.0625 ;
			if (*Temperature <= -39.99 || *Temperature > 200.)
				ok = 0 ;					// can't happen!
			break ;
			}
		case HUMIDITY1_DEVICE:
		case TEMPERATURE5_DEVICE: {
			const int bufSize = 7 ;
			unsigned char buf[bufSize] ;
			// read temperature 
			if (GetValue(deviceNo, buf, bufSize) == 0) {
				ok = 0 ;
				break ;
				}

			int version = cwGetVersion(cwBasicObj, deviceNo) ;

			*timeID  = ((buf[0] & 0x3f) << 8) + buf[1] ;
			// int humi = (buf[2] << 8) + buf[3] ;
			int temp = (buf[4] << 8) + buf[5] ;
			int valid = ((buf[0] & 0xc0) == 0xc0) ;	// MSB = valid-bit
			if (version >= 5 && version < 48) {	// 12 bit SHT15
				if (valid)
					valid = ((buf[4] & 0x80) == 0) ;	// MSB must be 0
				if (valid)
					valid = (buf[4] != 0) ;				// if value is > 0x100 (temp=-37,5C) there must be an error
				}
			if (!valid) { // invalid time
				ok = 0 ;
				break ;
				}
		//	double humidity = -4. + 0.0405 * humi - 2.8 * humi * humi / 1000000 ;
			if (version < 5)		// 14 bit
				*Temperature = -40. + 0.01 * temp ;
			else if (version < 48)				// 12 bit SHT 1x
				*Temperature = -40.1 + 0.04 * temp ;
			else				// 16 bit SHT 3x
				*Temperature = -45. + 175. * (temp / 65535.) ;
			if (*Temperature <= -39.99 || *Temperature > 200.)
				ok = 0 ;					// can't happen!
			break ;
			}
		default:
			ok = 0 ;
			break ;
		}

	if (ok && deviceNo < maxDevs) {
		double t = lastTemperature[deviceNo] ;
		if (t > -199.) {
			if (*Temperature < t - 1. || *Temperature > t + 1.)	// this should be measured twice
				ok = 0 ;
			}
		lastTemperature[deviceNo] = *Temperature ;
		}

	return ok ;
	}

int 
CUSBaccess::GetHumidity(int deviceNo, double *Humidity, int *timeID) {
	int ok = 1 ;

	switch (cwGetUSBType(cwBasicObj, deviceNo)) {
		case HUMIDITY1_DEVICE: {
			const int bufSize = 7 ;
			unsigned char buf[bufSize] ;
			// read temperature 
			if (GetValue(deviceNo, buf, bufSize) == 0) {
				ok = 0 ;
				break ;
				}

			int version = cwGetVersion(cwBasicObj, deviceNo) ;

			*timeID  = ((buf[0] & 0x3f) << 8) + buf[1] ;
			int humi = (buf[2] << 8) + buf[3] ;
			int temp = (buf[4] << 8) + buf[5] ;
			int valid = ((buf[0] & 0xc0) == 0xc0) ;	// MSB = valid-bit
			if (valid && version < 48)
				valid = ((buf[2] & 0x80) == 0) ;	// MSB must be 0
			if (!valid) { // invalid time
				ok = 0 ;
				break ;
				}
		//	old software before 2019
			if (version < 5)		// 12 bit
				*Humidity = -4. + 0.0405 * humi - 2.8 * humi * humi / 1000000 ;
			else					//  8 bit
				*Humidity = -4. + 0.648 * humi - 7.2 * humi * humi / 10000 ;
			// end of old software
			
			if (version < 5)		// 12 bit
				*Humidity = -4. + 0.0405 * humi - 2.8 * humi * humi / 1000000 ;
			else if (version < 48) {					//  8 bit  -  newer sensor, need new formular!
				double t1 = -40.1 + 0.04 * temp ;
				double h1 = -2.0468 + 0.5872 * humi - 4.0845 * humi * humi / 10000 ;
				*Humidity = h1 + (t1 - 25.) * (0.01 + 0.00128 * humi) ;
				}
			else {					//  16 bit  SHT 3x
				*Humidity = 100. * humi / 65535. ;
				}
			if (*Humidity < 0.) {
				*Humidity = 0. ;		// this is possible in rare cases
				}
			if (*Humidity > 99.) {
				*Humidity = 100. ;		// according to manual
				}

			break ;
			}
		default:
			ok = 0 ;
			break ;
		}
	return ok ;
	}
	

int 
CUSBaccess::SelectADC(int deviceNo, int subDevice) {
	static int sequenceNumber = 1 ;
	int rval = sequenceNumber ;
	const int bufSize = 6 ;
	unsigned char buf[bufSize] ;

		buf[0] = GetInfo ;
		buf[1] = subDevice ;
		buf[2] = sequenceNumber ;
		SetValue(deviceNo, buf, 3) ;
		if (++sequenceNumber >= 128)
			sequenceNumber = 1 ;

	return rval ;
	}

float 
CUSBaccess::GetADC(int deviceNo, int sequenceNumber, int subDevice) {
	int ok = 0 ;
	const int bufSize = 8 ;
	unsigned char buf[bufSize] ;
	float ftemp = -200. ;
	int adcVal = 0 ;

/*
#ifdef INOAGE		// Inoage just uses the Luminus, which 
	if (subDevice != 0)
		return -200. ;
#endif
	if (cwGetVersion(cwBasicObj, deviceNo) >= 0x10) {
		for (int timeout=50 ; timeout > 0 ; timeout--) {
			if (GetValue(deviceNo, buf, 6)) {
				if ((buf[0] & 0xc0) != 0xc0) {	// valid bit + ADC bit
	//				Sleep(25) ;
					continue ;
					}
				if (sequenceNumber != 0 && buf[1] != sequenceNumber)	// just to be sure
					continue ;				// no sleep, just kill the fifo
				ok = 1 ;
				break ;
				}
			}
		if (ok) {
			if (subDevice == 0)	
				adcVal = (buf[2] << 8) + buf[3] ;
			else
				adcVal = (buf[4] << 8) + buf[5] ;
			ftemp = adcVal / 40.95 ;		// * 100 / 4095 (2**12-1)
			}
		}
	else {
		for (int timeout=50 ; timeout > 0 ; timeout--) {
			if (GetValue(deviceNo, buf, 4)) {
				if ((buf[0] & 0xc0) != 0xc0) {	// valid bit + ADC bit
	//				Sleep(25) ;
					continue ;
					}
				if ((buf[0] & 7) != subDevice)	// not the right device
					continue ;				// no sleep, just kill the fifo
				if (buf[1] != sequenceNumber)
					continue ;				// no sleep, just kill the fifo
				ok = 1 ;
				break ;
				}
			}
		if (ok) {
			adcVal = (buf[2] << 8) + buf[3] ;
			ftemp = adcVal / 40.95 ;		// * 100 / 4095 (2**12-1)
			}
		}
*/
	int version = cwGetVersion(cwBasicObj, deviceNo) ;
	if (version >= 0x10) {
		int usedBufSize = 8 ;
		if (version < 0x13)
			usedBufSize = 6 ;
		for (int timeout=50 ; timeout > 0 ; timeout--) {
			if (GetValue(deviceNo, buf, usedBufSize)) {
				if ((buf[0] & 0xc0) != 0xc0) {	// valid bit + ADC bit
	//				Sleep(25) ;
					continue ;
					}
				if (sequenceNumber != 0 && buf[1] != sequenceNumber)	// just to be sure
					continue ;				// no sleep, just kill the fifo
				ok = 1 ;
				break ;
				}
			}
		if (ok) {
			if(cwGetADCtype(cwBasicObj, deviceNo) == 1) {		// hx71 device, 24 bit
				adcVal = (buf[3] << 16) + (buf[4] << 8) + buf[5] ;
				if (buf[3] & 0x80)	// negative 2er complement
					adcVal =  - ( 1 + ((~adcVal) & 0xffffff) ) ;

				ftemp = adcVal / 167772.15 ;		// * 100 / 16777215 (2**24-1)
				}
			else if (cwGetADCtype(cwBasicObj, deviceNo) == 2) {		// PT100 ADC
				if (subDevice == 0)	
					adcVal = (buf[2] << 8) + buf[3] ;
				ftemp = adcVal / 40.95 ;		// * 100 / 4095 (2**12-1)
				}
			else {									// standard 12 bit ADC
				if (subDevice == 0)	
					adcVal = (buf[2] << 8) + buf[3] ;
				else if (subDevice == 1)	
					adcVal = (buf[4] << 8) + buf[5] ;
				else
					adcVal = (buf[6] << 8) + buf[7] ;
				ftemp = adcVal / 40.95 ;		// * 100 / 4095 (2**12-1)
				}
			}
		}
	else {
		for (int timeout=50 ; timeout > 0 ; timeout--) {
			if (GetValue(deviceNo, buf, 4)) {
				if ((buf[0] & 0xc0) != 0xc0) {	// valid bit + ADC bit
	//				Sleep(25) ;
					continue ;
					}
				if ((buf[0] & 7) != subDevice)	// not the right device
					continue ;				// no sleep, just kill the fifo
				if (buf[1] != sequenceNumber)
					continue ;				// no sleep, just kill the fifo
				ok = 1 ;
				break ;
				}
			}
		if (ok) {
			adcVal = (buf[2] << 8) + buf[3] ;
			ftemp = adcVal / 40.95 ;		// * 100 / 4095 (2**12-1)
			}
		}

	return ftemp ;
	}

int
CUSBaccess::IsAmpel(int deviceNo) {	// return true if this is a traffic light device
	int rval = cwIsAmpel(cwBasicObj, deviceNo) ;

	return rval ;
	}

int		// return 0 for pre 2014 designed devices, 13 for new devices
CUSBaccess::GetHWversion(int deviceNo) {
	int rval = cwGetHWversion(cwBasicObj, deviceNo) ;

	return rval ;
	}

int	
CUSBaccess::IsIdeTec(int deviceNo) {

	int rval = 0 ;		// IdeTec is not handled here

	return rval ;
	}

// returns data if ok or -1 in case of an error
int		
CUSBaccess::IOX(int deviceNo, int addr, int data) {
	int rval = cwIOX(cwBasicObj, deviceNo, addr, data) ;

	return rval ;
	}


void
CUSBaccess::DebugWrite(char *s) { 
	cwDebugWrite(s) ;
	}

void
CUSBaccess::DebugWrite(char *f, int a1) {
	static char s[1024] ;
	sprintf(s, f, a1) ;
	cwDebugWrite(s) ;
	}

void
CUSBaccess::DebugWrite(char *f, int a1,int a2) { 
	static char s[1024] ;
	sprintf(s, f, a1, a2) ;
	cwDebugWrite(s) ;
	}

void
CUSBaccess::DebugWrite(char *f, int a1, int a2, int a3) { 
	static char s[1024] ;
	sprintf(s, f, a1, a2, a3) ;
	cwDebugWrite(s) ;
	}

void
CUSBaccess::DebugWrite(char *f, int a1, int a2, int a3, int a4) { 
	static char s[1024] ;
	sprintf(s, f, a1, a2, a3, a4) ;
	cwDebugWrite(s) ;
	}
