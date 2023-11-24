//				 mit Argument  schalte den angeschlossenen Schalter 0=aus, 1=ein.
//	special knoxBox version

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "USBaccess.h"

int 
main(int argc, char* argv[]) {
	CUSBaccess CWusb ;

	int USBcount = CWusb.OpenCleware() ;
	printf("OpenCleware found %d devices\n", USBcount) ;
	int devID ;
	for (devID=0 ; devID < USBcount ; devID++) {
		// get some more info for switch devices
		int devType = CWusb.GetUSBType(devID) ;
		int sernum = CWusb.GetSerialNumber(devID) ;
		printf("%d: devType=%d, sernum=%d ", devID, devType, sernum) ;
		if (devType == CUSBaccess::SWITCH1_DEVICE) {
			if ( (sernum >= 1500000 && sernum < 1600000) || (sernum >= 1750000 && sernum < 1800000) ) {	// Cutter, Connect or Multi2
				int cutterType = CWusb.IOX(devID, 2, -1) ;
				if (cutterType == -1)		// retry
					cutterType = CWusb.IOX(devID, 2, -1) ;
				switch (cutterType) {
					case 1:
						printf("USB-Cutter detected\n") ;
						break ;
					case 2:
						printf("USB-Connect detected\n") ;
						break ;
					case 3:
						printf("USB-Multi2 detected\n") ;
						break ;
					case 4:
						printf("USB-Multi2x detected\n") ;
						break ;
					default:
						printf("unknown device %d detected\n", cutterType) ;
						break ;
					}
				}
			else {
				printf("USB-Switch detected\n") ;
				}
			}
		}
	
	// you should see a list of devices with the parameter you requested
	// now you may read a channel number (Channel) and 0/1 for on off (onOff)
	int Channel = 0 ;
	int onoff = 1 ;
	
	// some code to choose the device to switch
	
	if (Channel >= 0 && Channel < USBcount)
		CWusb.SetSwitch(Channel, CUSBaccess::SWITCH_0, onoff) ;
			
	return 0 ;
	}
			
	

	