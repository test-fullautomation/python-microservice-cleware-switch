// Basic class implementation for access to USB HID devices
//
// (C) 2001 Copyright Cleware GmbH
// All rights reserved
//
// History:
// 05.01.01	ws	Initial coding
// 01.11.01	ws	Linux coding
// ...
// 25.05.13 ws	new controller support
// 15.05.14 ws	new controller needs special serial number handling
// 12.10.18 ws	fixed bug in device detection


#define HID_MAX_USAGES 1024		// see /usr/src/linux/drivers/usb/input/hid.h

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
// ubuntu #include <linux/usb.h>
#include <linux/hiddev.h>

#include "USBaccessBasic.h"

const int maxHID = 128 ;
cwSUSBdata data[128] ;

void
cwInitCleware() {
	int h ;

	for (h=0 ; h < maxHID ; h++)
		data[h].handle = INVALID_HANDLE_VALUE ;
	}

void
cwCloseCleware() {
	int h ;
	for (h=0 ; h < maxHID  ; h++) {
		if (data[h].handle != INVALID_HANDLE_VALUE) {
			close(data[h].handle) ;
			data[h].handle = INVALID_HANDLE_VALUE ;
			}
		}
	}


// returns number of found Cleware devices
int
cwOpenCleware() {
	int ok = 1 ;	
	int i, h ;
	int handleCount = 0 ;
	char *hiddevname[] = { "/dev/usb/hiddev", "/dev/bus/usb/hiddev" } ;
	int hId ;

	for (h=0 ; h < handleCount ; h++) {
		if (data[h].handle != INVALID_HANDLE_VALUE) {
			close(data[h].handle) ;
			data[h].handle = INVALID_HANDLE_VALUE ;
			}
		}

	for (hId=i=0 ; i < 16 ; i++) {		// Linux supports up to 16 HID devices
		struct hiddev_devinfo dinfo ;
		struct hiddev_report_info rinfo ;
		char devname[32] ;
		int tryNextOne = 0 ;	// if 1 skip current device and try the next one

		sprintf(devname, "%s%d", hiddevname[hId], i) ;
		data[handleCount].handle = open(devname, O_RDWR) ;
//			printf("open %s returns %d\n", devname, data[handleCount].handle) ;
		if (hId == 0 && i == 0 && data[0].handle == INVALID_HANDLE_VALUE) {
			hId++ ; 	// maybe the hiddev directroy is the wrong one
			i = 0 ;
			continue ;
			}
		if (data[handleCount].handle >= 0) {
			ok = ioctl(data[handleCount].handle, HIDIOCGDEVINFO, (void *)&dinfo) ;
			if (ok < 0)
				tryNextOne = 1 ;
			else if (dinfo.vendor != 0x0d50)
				tryNextOne = 1 ;

			if (ok >= 0 && !tryNextOne) {
				data[handleCount].gadgettype = (enum USBtype_enum)dinfo.product ;
				data[handleCount].gadgetVersionNo = dinfo.version ;
				data[handleCount].HWversion = 0 ;
				data[handleCount].isAmpel = 0 ; 
				ok = ioctl(data[handleCount].handle, HIDIOCAPPLICATION, 0) ;
				if (ok == -1)
					tryNextOne = 1 ;
				else
					ok = 0 ;
				}

			if (ok >= 0 && !tryNextOne) {
				static char strbuf[261] ; // 256 + sizeof(int) + 1 ;
				*(int *) strbuf = 3 ;
				ok = ioctl(data[handleCount].handle, HIDIOCGSTRING, (void *)&strbuf) ;
				if (ok < 0)
					tryNextOne = 1 ;
				else {
					int SerNum=0 ;
					char *s=strbuf+sizeof(int) ;
					int strInc = (s[1] == 0) ? 2 : 1 ;
					for ( ; *s ; s+=strInc) {	// unicode byte 2 == 0
						if (*s >= '0' && *s <= '9')
							SerNum = SerNum * 16 + *s - '0' ;
						else if (*s >= 'A' && *s <= 'F')
							SerNum = SerNum * 16 + *s - 'A' + 10 ;
						}
					data[handleCount].report_type = HID_REPORT_ID_FIRST ;
					if (SerNum == 0x63813) {	// this is the next controller - get serial number directly
						data[handleCount].HWversion = 13 ;
						SerNum = -1 ;
						}
					if (SerNum <= 0) {		// getting the Serial number failed, so get it directly!
							SerNum = 0 ;
							int addr ;
							for (addr=8 ; addr <= 14 ; addr++) {	// unicode byte 2 == 0
								int data = cwIOX(handleCount, addr, -1) ;
								if (data >= '0' && data <= '9')
									SerNum = SerNum * 16 + data - '0' ;
								else if (data >= 'A' && data <= 'F')
									SerNum = SerNum * 16 + data - 'A' + 10 ;
								else {
									SerNum = -1 ;		// failed!
									break ;
									}
								}
							}
					data[handleCount].SerialNumber = SerNum ;
					if (data[handleCount].gadgettype == SWITCH1_DEVICE && data[handleCount].HWversion == 13) {
						int d2 = cwIOX(handleCount, 2, -1) ;
						if (d2 & 0x20)
							data[handleCount].isAmpel = 1 ; 
						d2 &= 0x0f ;
						if (d2 == 0)
							data[handleCount].gadgettype = WATCHDOG_DEVICE ;
						else if (d2 == 1)
							data[handleCount].gadgettype = AUTORESET_DEVICE ;
						}
					}
				}					

			if (ok >= 0 && !tryNextOne)
				handleCount++ ;
			else {	// not ok - close handle
				close(data[handleCount].handle) ;
				data[handleCount].handle = INVALID_HANDLE_VALUE ;
				}

			}
		}

	return handleCount ;
	}

// try to find disconnected devices - returns true if succeeded
int
cwRecover(int devNum) {
	int ok = 1 ;	
	int i, h ;
	int reconnectOk = 0 ;
	cwSUSBdata USBdata ;

	if (data[devNum].handle != INVALID_HANDLE_VALUE) {
		close(data[devNum].handle) ;
		data[devNum].handle = INVALID_HANDLE_VALUE ;
		}

	for (i=0 ; i < 16 ; i++) {		// Linux supports up to 16 HID devices
		struct hiddev_devinfo dinfo ;
		struct hiddev_report_info rinfo ;
		const char hiddevname[] = "/dev/usb/hiddev" ;
		char devname[32] ;
		int tryNextOne = 0 ;	// if 1 skip current device and try the next one

		sprintf(devname, "%s%d", hiddevname, i) ;
		USBdata.handle = open(devname, O_RDWR) ;
		if (USBdata.handle >= 0) {
			ok = ioctl(USBdata.handle, HIDIOCGDEVINFO, (void *)&dinfo) ;
			if (ok < 0)
				tryNextOne = 1 ;
			else if (dinfo.vendor != 0x0d50)
				tryNextOne = 1 ;

			if (ok >= 0 && !tryNextOne) {
				USBdata.gadgettype = (enum USBtype_enum)dinfo.product ;
				USBdata.gadgetVersionNo = dinfo.version ;
				ok = ioctl(USBdata.handle, HIDIOCAPPLICATION, 0) ;
				if (ok == -1)
					tryNextOne = 1 ;
				else
					ok = 0 ;
				}
			if (USBdata.gadgettype != data[devNum].gadgettype || USBdata.gadgetVersionNo != data[devNum].gadgetVersionNo) 
				tryNextOne = 1 ;

			if (ok >= 0 && !tryNextOne) {
				static char strbuf[261] ; // 256 + sizeof(int) + 1 ;
				*(int *) strbuf = 3 ;
				ok = ioctl(USBdata.handle, HIDIOCGSTRING, (void *)&strbuf) ;
				if (ok >= 0) {
					int SerNum=0 ;
					char *s=strbuf+sizeof(int) ;
					int strInc = (s[1] == 0) ? 2 : 1 ;
					for ( ; *s ; s+=strInc) {	// unicode byte 2 == 0
						if (*s >= '0' && *s <= '9')
							SerNum = SerNum * 16 + *s - '0' ;
						else if (*s >= 'A' && *s <= 'F')
							SerNum = SerNum * 16 + *s - 'A' + 10 ;
						}
					if (SerNum == 0x63813) {	// this is the next controller - get serial number directly
						data[devNum].handle = USBdata.handle ;
						data[devNum].report_type = HID_REPORT_ID_FIRST ;
						SerNum = -1;
						}
					if (SerNum <= 0) {	// getting the Serial number failed, so get it directly!
						SerNum = 0 ;
						int addr ;
						for (addr=8 ; addr <= 14 ; addr++) {	// unicode byte 2 == 0
							int data = cwIOX(devNum, addr, -1) ;
							if (data >= '0' && data <= '9')
								SerNum = SerNum * 16 + data - '0' ;
							else if (data >= 'A' && data <= 'F')
								SerNum = SerNum * 16 + data - 'A' + 10 ;
							else {
								SerNum = -1 ;		// failed!
								break ;
								}
							}
						}

					USBdata.SerialNumber = SerNum ;
					if (USBdata.SerialNumber == data[devNum].SerialNumber) {
						data[devNum].handle = USBdata.handle ;
						data[devNum].report_type = HID_REPORT_ID_FIRST ;
						reconnectOk = 1;
						break ;
						}
					data[devNum].handle = INVALID_HANDLE_VALUE ;
					}
				}
			close(USBdata.handle) ;
			USBdata.handle = INVALID_HANDLE_VALUE ;
			}
		}

	return reconnectOk ;
	}

// returns 1 if ok or 0 in case of an error
int		
cwGetValue(int deviceNo, int UsagePage, int Usage, unsigned char *buf, int bufsize) {
	// UsagePage and Usage needed for win32
	struct hiddev_field_info finfo ;
	struct hiddev_usage_ref uref ;
	struct hiddev_report_info rinfo ;
	int ok = 1 ;

	if (deviceNo < 0 || deviceNo >= maxHID || data[deviceNo].handle == INVALID_HANDLE_VALUE)
		return 0 ;		// out of range

	if (data[deviceNo].report_type == HID_REPORT_ID_FIRST) {	// doing this the first time
		ok = ioctl(data[deviceNo].handle, HIDIOCAPPLICATION, 0) ;
		if (ok != -1) {
			rinfo.report_id = 0 ;
			rinfo.report_type = HID_REPORT_TYPE_INPUT ;
			ok = ioctl(data[deviceNo].handle, HIDIOCGREPORT, (void *)&rinfo) ;
			}

		if (ok >= 0) {
			rinfo.report_id = HID_REPORT_ID_FIRST ;		// only one report
			rinfo.report_type = HID_REPORT_TYPE_INPUT ;
			ok = ioctl(data[deviceNo].handle, HIDIOCGREPORTINFO, (void *)&rinfo) ;
			}
		if (ok >= 0) {
			finfo.report_type = rinfo.report_type ;
			finfo.report_id = rinfo.report_id ;
			finfo.field_index = 0 ;
			ok = ioctl(data[deviceNo].handle, HIDIOCGFIELDINFO, (void *)&finfo) ;
			}
		if (ok >= 0) {
			uref.report_type = finfo.report_type ;
			uref.report_id = finfo.report_id ;
			uref.field_index = 0 ;
			uref.usage_index = 0 ;
			ok = ioctl(data[deviceNo].handle, HIDIOCGUCODE, (void *)&uref) ;
			}
		if (ok >= 0 && finfo.maxusage != bufsize) {
			// printf("bufsize mismatch maxusage=%d\n", finfo.maxusage) ;
			ok = -1 ;
			}
		if (ok >= 0)
			data[deviceNo].report_type = rinfo.report_type ;
		}
	else {
		uref.report_type = data[deviceNo].report_type ;
		uref.report_id = 0 ;
		uref.field_index = 0 ;
		uref.usage_index = 0 ;
		}
	if (ok >= 0) {
		int u ;
		for (u=0 ; u < bufsize ; u++) {
			uref.usage_index = u ;
			ok = ioctl(data[deviceNo].handle, HIDIOCGUSAGE, (void *)&uref) ;
			if (ok < 0) {
				// perror("HIDIOCGUSAGE failed - ") ;
				break ;
				}
			buf[u] = uref.value & 0xff ;
			}
		}
	return (ok >= 0) ? 1 : 0 ;
	}


int 
cwSetValue(int deviceNo, int UsagePage, int Usage, unsigned char *buf, int bufsize) {
	// UsagePage and Usage needed for win32
	struct hiddev_report_info rinfo ;
	struct hiddev_field_info finfo ;
	struct hiddev_usage_ref uref ;
	int ok  = 0 ;

	if (deviceNo < 0 || deviceNo >= maxHID || data[deviceNo].handle == INVALID_HANDLE_VALUE)
		return 0 ;		// out of range

	rinfo.report_id = HID_REPORT_ID_FIRST ;		// only one report
	rinfo.report_type = HID_REPORT_TYPE_OUTPUT ;
	ok = ioctl(data[deviceNo].handle, HIDIOCGREPORTINFO, (void *)&rinfo) ;
	if (ok >= 0) {
		finfo.report_type = rinfo.report_type ;
		finfo.report_id = rinfo.report_id ;
		finfo.field_index = 0 ;
		ok = ioctl(data[deviceNo].handle, HIDIOCGFIELDINFO, (void *)&finfo) ;
		}
	if (ok >= 0) {
		uref.report_type = finfo.report_type ;
		uref.report_id = finfo.report_id ;
		uref.field_index = 0 ;
		uref.usage_index = 0 ;
		ok = ioctl(data[deviceNo].handle, HIDIOCGUCODE, (void *)&uref) ;
		}
	if (ok >= 0 && finfo.maxusage != bufsize) {
		// printf("bufsize mismatch maxusage=%d\n", finfo.maxusage) ;
		ok = -1 ;
		}
	if (ok >= 0) {
		int u ;
		for (u=0 ; u < finfo.maxusage ; u++) {
			uref.value = buf[u] ;
			uref.usage_index = u ;
			ok = ioctl(data[deviceNo].handle, HIDIOCSUSAGE, (void *)&uref) ;
			if (ok < 0)
				break ;
			}
		}
	if (ok >= 0) {
		ok = ioctl(data[deviceNo].handle, HIDIOCSREPORT, (void *)&finfo) ;
		}

	return (ok >= 0) ? 1 : 0 ;
	}

unsigned long int
cwGetHandle(int deviceNo) { 
	unsigned long int rval = INVALID_HANDLE_VALUE ;

	if (deviceNo >= 0 && deviceNo < maxHID)
		rval = data[deviceNo].handle ;

	return rval ; 
	}

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

	buf[0] = EEread ;
	if (sixteenbit) {
		buf[1] = 0 ;	// high byte 0
		buf[2] = addr ;
		buf[3] = 0 ;
		cwSetValue(deviceNo, 65441, 4, buf, 4) ;
		bufsize = 7 ;
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
			if ((buf[0] & 0x80) == 0) {
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
			else {
				Xaddr = buf[4] ;
				Xdata = buf[5] ;
				}
			if (sixteenbit) {
				Xaddr = (Xaddr << 8) + buf[5] ;
				Xdata = buf[6] ;
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
