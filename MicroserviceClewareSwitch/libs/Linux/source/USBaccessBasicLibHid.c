// Basic class implementation for access to USB HID devices
//
// (C) 2001 Copyright Cleware GmbH
// All rights reserved
//
// History:
// 05.01.01	ws	Initial coding
// 01.11.01	ws	Linux coding
// ...
// 30.10.12	fvh	libhid version		/* libhidapi must be installed */
// 25.05.13 ws	new controller support
// 15.05.14 ws	new controller needs special serial number handling


#define HID_MAX_USAGES 1024		// see /usr/src/linux/drivers/usb/input/hid.h

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
// ubuntu #include <linux/usb.h>
#include <linux/hiddev.h>

#include <hidapi/hidapi.h>

#include "USBaccessBasic.h"

const int maxHID = 256 ;
cwSUSBdata data[256] ;

void
cwInitCleware() {
	int h ;

	for (h=0 ; h < maxHID ; h++)
		data[h].handle = INVALID_HANDLE_VALUE ;
		
	if (hid_init() == -1)
		fprintf(stderr, "Error initializing HID library\n");
	}

void
cwCloseCleware() {
	int h ;
	for (h=0 ; h < maxHID  ; h++) {
		if (data[h].handle != INVALID_HANDLE_VALUE) {
			hid_close(data[h].handle) ;
			data[h].handle = INVALID_HANDLE_VALUE ;
			}
		}
	}


// returns number of found Cleware devices
int
cwOpenCleware() {
	int h ;
	int handleCount = 0 ;

	for (h=0 ; h < maxHID ; h++) {
		if (data[h].handle != INVALID_HANDLE_VALUE) {
			hid_close(data[h].handle) ;
			data[h].handle = INVALID_HANDLE_VALUE ;
			}
		}

	struct hid_device_info *devs = NULL, *cur_dev = NULL;

	cur_dev = devs = hid_enumerate(0x0d50, 0);

	while (cur_dev) {		
		int SerNum = -1 ;
		data[handleCount].vendorID = cur_dev->vendor_id;
		data[handleCount].productID = cur_dev->product_id;
//		data[handleCount].hidpath = strdup(cur_dev->path);
		data[handleCount].gadgettype = (enum USBtype_enum)data[handleCount].productID ;
		data[handleCount].gadgetVersionNo = cur_dev->release_number; ;
		data[handleCount].HWversion = 13 ;		// ignore old devices
		data[handleCount].isAmpel = 0 ; 
		data[handleCount].handle = hid_open_path(cur_dev->path) ;
		if (data[handleCount].handle == 0)
			printf("hid_open_path failed, path = %s\n", cur_dev->path ) ;
		if (cur_dev->serial_number != NULL) {
	    char buffer[256] = { 0 };
	    wcstombs(buffer, cur_dev->serial_number, wcslen(cur_dev->serial_number));
	    SerNum = strtol(buffer, NULL, 16) ;
	  	}

		data[handleCount].report_type = 0 ; // HID_REPORT_ID_FIRST ;
		if (SerNum == 0x63813) {	// this is the next controller - get serial number directly
			data[handleCount].HWversion = 13 ;
			SerNum = -1 ;
			}
		if (SerNum <= 0) {		// getting the Serial number failed, so get it directly!
			SerNum = 0 ;
			int addr ;
			for (addr=8 ; addr <= 14 ; addr++) {	// unicode byte 2 == 0
				int datum = cwIOX(handleCount, addr, -1) ;
				if (datum >= '0' && datum <= '9')
					SerNum = SerNum * 16 + datum - '0' ;
				else if (datum >= 'A' && datum <= 'F')
					SerNum = SerNum * 16 + datum - 'A' + 10 ;
				else {
					SerNum = -1 ;		// failed!
					break ;
					}
				}
			}
			
		data[handleCount].SerialNumber = SerNum ;
			
		if (data[handleCount].gadgettype == SWITCH1_DEVICE && data[handleCount].HWversion == 13 &&
			 ! ((SerNum >= 1500000 && SerNum < 1600000) || (SerNum >= 1750000 && SerNum < 1800000)) ) {		// not for Cutter&Co.
			int d2 = cwIOX(handleCount, 2, -1) ;
			if (d2 & 0x20)
				data[handleCount].isAmpel = 1 ; 
			d2 &= 0x0f ;
			if (d2 == 0)
				data[handleCount].gadgettype = WATCHDOG_DEVICE ;
			else if (d2 == 1)
				data[handleCount].gadgettype = AUTORESET_DEVICE ;
			}
		cur_dev = cur_dev->next;
		handleCount++ ;
		}

	return handleCount ;
	}

// try to find disconnected devices - returns true if succeeded
int
cwRecover(int devNum) {
	
	return 0 ;		// not used here
	}

unsigned char seqNum = 0 ;

// returns 1 if ok or 0 in case of an error
int		
cwGetValue(int deviceNo, int UsagePage, int Usage, unsigned char *buf, int bufsize) {
	int ok = 1 ;

	if (deviceNo < 0 || deviceNo >= maxHID || data[deviceNo].handle == INVALID_HANDLE_VALUE)
		ok = 0 ;		// out of range

	unsigned char lbuf[3] = { 0x00, (seqNum++), 0x81 };
	if (ok && hid_send_feature_report(data[deviceNo].handle, lbuf, sizeof lbuf) < 0)
		ok = 0 ;

	if (ok && hid_read(data[deviceNo].handle, buf, bufsize) < 0)
		ok = 0 ;

	return ok ;
	}


int 
cwSetValue(int deviceNo, int UsagePage, int Usage, unsigned char *buf, int bufsize) {
	int ok = 1 ;
	const int maxP1 = 16 ;
	unsigned char p1[maxP1] ;
	
	if (deviceNo < 0 || deviceNo >= maxHID || data[deviceNo].handle == INVALID_HANDLE_VALUE || bufsize >= maxP1)
		ok = 0 ;		// out of range

	if (ok) {
		unsigned char *s=buf, *d=p1+1 ;
		for (int i=0 ; i < bufsize ; i++)
			*d++ = *s++ ;
		p1[0] = 0 ;
		
		if (hid_write(data[deviceNo].handle, (unsigned char *)p1, bufsize + 1) < 0)
			ok = 0 ;
		}
		
	return ok ;
	}

/*
unsigned long int
cwGetHandle(int deviceNo) { 
	unsigned long int rval = INVALID_HANDLE_VALUE ;

	if (deviceNo >= 0 && deviceNo < maxHID)
		rval = data[deviceNo].handle ;

	return rval ; 
	}
*/

int
cwGetVersion(int deviceNo) { 
	int rval ;

	if (deviceNo < 0 || deviceNo >= maxHID || data[deviceNo].handle == INVALID_HANDLE_VALUE)
		rval = -1 ;
	else
		rval = data[deviceNo].gadgetVersionNo ;

	return rval ; 
	}

int
cwGetSerialNumber(int deviceNo) { 
	int rval ;

	if (deviceNo < 0 || deviceNo >= maxHID || data[deviceNo].handle == INVALID_HANDLE_VALUE)
		rval = -1 ;
	else
		rval = data[deviceNo].SerialNumber ;

	return rval ; 
	}

enum USBtype_enum
cwGetUSBType(int deviceNo) { 
	enum USBtype_enum rval ;

	if (deviceNo < 0 || deviceNo >= maxHID || data[deviceNo].handle == INVALID_HANDLE_VALUE)
		rval = ILLEGAL_DEVICE ;
	else
		rval = data[deviceNo].gadgettype ;

	return rval ; 
	}

int	
cwGetHWversion(int deviceNo) {			// return current
	int rval = 0 ;

	if (deviceNo < 0 || deviceNo >= maxHID || data[deviceNo].handle == INVALID_HANDLE_VALUE)
		rval = -1 ;
	else
		rval = data[deviceNo].HWversion ;

	return rval ; 
	}

int	
cwIsAmpel(int deviceNo) {
	int rval = 0 ;

	if (deviceNo < 0 || deviceNo >= maxHID || data[deviceNo].handle == INVALID_HANDLE_VALUE)
		rval = -1 ;
	else
		rval = data[deviceNo].isAmpel ;

	return rval ; 
	}

int
cwValidSerNum(int SerialNumber, enum USBtype_enum devType) {
	static int outdated[] = {
		54,59,60,62,63,64,65,66,67,68,69,
		72,74,75,76,77,82,83,85,87,88,89,
		90,91,92,93,95,96,98,99,
		100,101,102,103,105,106,107,108,109,110,
		113,116,117,119,120,122,124,125,126,128,
		131,132,135,139,140,142,143,145,147,148,149,
		150,151,152,153,154,155,156,157,160,161,162,163,168,169,
		170,171,172,173,174,175,176,180,184,187,188,189,190,191,192,193,195,197,198,
		201,203,204, 205, 206, 219,220,221,260,272,273,274,275,276,278,279,
		280,281,416,
		5002,5003,5004,5005,5006,5007,5008,5010,
		5011,5012,5013,5014,5015,5016,
		5017,5018,5019,5020,5021,5022,5023,5024,
		5025,5026,5028,5029,5032,5033,5034,
		5035,5036,5041,5043,5044,5046,5049,5050,
		5052,5053,5055,5057,5071,5073,5076,5089,5091,5101,5102,5103,
		5104,5106,5109,5114,5116,5117,5118,5119,
		5120,5121,5122,5147,5163,5164,7502,7503,
		7504,7505,7511,7513,8192,8193,8194,
		8503,8504,8505,8506,8507,8508,8509,8510,8511,8512,8513
		} ;

	static int outdatedSwitches[] = {		// double numbers!!
		510,511,513,514,517,518,520,532			//	Switches
//		, 5314	// test
		} ;

	int rval = 1 ;

	int size = sizeof(outdated) / sizeof(int) ;
	int *pt = &(outdated[0]) ;
	int i ;

	for (i=0 ; i < size ; i++, pt++) {
		if (SerialNumber == *pt) {
			rval = 0 ;
			break ;
			}
		}
	if (rval == 1 && devType == SWITCH1_DEVICE) {
		size = sizeof(outdatedSwitches) / sizeof(int) ;
		pt = &(outdatedSwitches[0]) ;
		for (i=0 ; i < size ; i++, pt++) {
			if (SerialNumber == *pt) {
				rval = 0 ;
				break ;
				}
			}
		}

	return rval ; 
	}

void
cwDebugWrite(char *s) { 
	fputs(s, stderr) ;
	}

void
cwDebugClose() { 
	}

int	
cwIOX(int deviceNo, int addr, int datum) {	// return datum if ok, datum=-1=Read operation
	const int maxbufsize = 8 ;
	int bufsize = 6 ;
	unsigned char buf[maxbufsize] ;
	int ok = 1 ;
	if (deviceNo < 0 || deviceNo >= maxHID || data[deviceNo].handle == INVALID_HANDLE_VALUE)
		return(-1) ;

	int devType = data[deviceNo].gadgettype ;
	int version = data[deviceNo].gadgetVersionNo ;
	int sixteenbit = (devType == TEMPERATURE2_DEVICE || devType == HUMIDITY1_DEVICE || devType == HUMIDITY2_DEVICE) ;

	if (datum >= 0) {		// -1 = Read command
		buf[0] = EEwrite ;
		if (sixteenbit) {
			buf[1] = addr >> 8 ;	// high byte 0
			buf[2] = addr ;
			buf[3] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 4) ;
			}
		else if (cw2IsIdeTec(devType, version) && version >= 0x8102) {
			buf[1] = addr  >> 8 ;	// high byte
			buf[2] = addr ;
			buf[3] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 6) ;
			}
		else if (devType == CONTACT00_DEVICE && version > 6) {
			buf[1] = addr ;
			buf[2] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 5) ;
			}
		else if (devType == DISPLAY_DEVICE) {
			buf[1] = addr ;
			buf[2] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 5) ;
			}
		else if (devType == WATCHDOGXP_DEVICE || devType == SWITCHX_DEVICE) {
			buf[1] = addr ;
			buf[2] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 5) ;
			}
		else if (devType == ENCODER01_DEVICE) {
			buf[1] = addr ;
			buf[2] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 6) ;
			}
		else if (devType == ADC0800_DEVICE) {
			buf[1] = addr ;
			buf[2] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 3) ;
			}
		else if (devType == POWER_DEVICE) {
			buf[1] = addr ;
			buf[2] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 3) ;
			}
		else if (devType == KEYC16_DEVICE || devType == KEYC01_DEVICE) {
			buf[1] = addr ;
			buf[2] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 5) ;
			}
		else if (devType == MOUSE_DEVICE) {
			buf[1] = addr ;
			buf[2] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 5) ;
			}
		else {
			buf[1] = addr ;
			buf[2] = datum ;
			cwSetValue(deviceNo, 65441, 4, buf, 3) ;
			}
		usleep(100*1000) ;
		}

	if (datum == -4)					// read 4 bytes in one step
		buf[0] = EEread4 ;		
	else
		buf[0] = EEread ;
		
	if (sixteenbit) {
		buf[1] = 0 ;	// high byte 0
		buf[2] = addr ;
		buf[3] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 4) ;
		bufsize = 7 ;
		}
	else if (cw2IsIdeTec(devType, version) && version >= 0x8102) {
		buf[1] = ( (addr >> 8) & 0xff) ;
		buf[2] = ( addr & 0xff ) ;
		cwSetValue(deviceNo, 65441, 4, buf, 6) ;
		}
	else if (devType == CONTACT00_DEVICE && version > 6) {
		buf[1] = addr ;
		buf[2] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 5) ;
		}
	else if (devType == DISPLAY_DEVICE) {
		buf[1] = addr ;
		buf[2] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 5) ;
		}
	else if (devType == WATCHDOGXP_DEVICE || devType == SWITCHX_DEVICE) {
		buf[1] = addr ;
		buf[2] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 5) ;
		}
	else if (devType == KEYC16_DEVICE || devType == KEYC01_DEVICE) {
		buf[1] = addr ;
		buf[2] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 5) ;
		bufsize = 8 ;
		}
	else if (devType == MOUSE_DEVICE) {
		buf[1] = addr ;
		buf[2] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 5) ;
		bufsize = 4 ;
		}
	else if (devType == ADC0800_DEVICE) {
		buf[1] = addr ;
		buf[2] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 3) ;
		bufsize = 4 ;
		}
	else if (devType == POWER_DEVICE) {
		buf[1] = addr ;
		buf[2] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 3) ;
		bufsize = 3 ;
		}
	else if (devType == ENCODER01_DEVICE) {
		buf[1] = addr ;
		buf[2] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 6) ;
		}
	else {
		buf[1] = addr ;
		buf[2] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 3) ;
		}

	usleep(10*1000) ;
	ok = 40 ;
	int Xdata = -1 ;
	while (ok) {
		if (cwGetValue(deviceNo, 65441, 3, buf, bufsize)) {
			unsigned char mask = 0x80 ;
			if (cw2IsIdeTec(devType, version) && version >= 0x8102)
				mask = 0x40 ;
			if ((buf[0] & mask) == 0) {
				if (--ok == 0) {
					// MessageBox("GetValue still not valid", "Error") ;
					break ;
					}
				else {
					usleep(10*1000) ;
					continue ;
					}
				}
			int Xaddr = 0 ;
			if (bufsize == 3 || devType == MOUSE_DEVICE) {
				Xaddr = buf[1] ;
				Xdata = buf[2] ;
				}
			else if (bufsize == 4) {
				Xaddr = buf[2] ;
				Xdata = buf[3] ;
				}
			else if (datum == -4) {	// read 4 bytes
				Xaddr = buf[5] ;
				Xaddr += (addr & 0xff00) ;			// don't care about the upper 8 bits
				Xdata = (buf[1] << 24) + (buf[2] << 16) + (buf[3] << 8) + buf[4] ;
				}
			else {
				Xaddr = buf[4] ;
				Xdata = buf[5] ;
				if (cw2IsIdeTec(devType, version) && version >= 0x8102) {
					Xaddr += (buf[3] << 8) ;
					}
				}
			if (Xaddr != addr) {
				if (--ok == 0) {
					// MessageBox("GetValue address error", "Error") ;
					break ;
					}
				else {
					usleep(10*1000) ;
					continue ;
					}
				}
			if (datum >= 0 && Xdata != datum) {
				if (--ok == 0) {
					// MessageBox("Write error", "Error") ;
					break ;
					}
				else {
					usleep(10*1000) ;
					continue ;
					}
				}
			break ;
			}
		else {
			if (--ok == 0) {
				// MessageBox("GetValue failed", "Error") ;
				break ;
				}
			else {
				usleep(10*1000) ;
				continue ;
				}
			}
		break ;		// read was ok
		}

	if (!ok)
		Xdata = -1 ;

	return Xdata ;
	}
