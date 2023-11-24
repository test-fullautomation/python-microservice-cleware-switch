// Example.cpp : no argument: get temperatur or humidity
//				 with argument  turn switch 0=off, 1=on.
//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "USBaccess.h"

int 
main(int argc, char* argv[]) {
	CUSBaccess CWusb ;

	printf("Start USB Access Example!\n") ;

	int USBcount = CWusb.OpenCleware() ;
	printf("OpenCleware found %d devices\n", USBcount) ;

	for (int devID=0 ; devID < USBcount ; devID++) {
		int devType = CWusb.GetUSBType(devID) ;
		printf("Device %d: Type=%d, Version=%d, SerNum=%d\n", devID,
					devType, CWusb.GetVersion(devID),
					CWusb.GetSerialNumber(devID)) ;
		printf("argc=%d\n", argc) ;
		if (argc == 2) {	// turn switch
			if (devType == CUSBaccess::SWITCH1_DEVICE) {
				printf("argv=%c <0x%02x>\n", argv[1][0], argv[1][0]) ;
				printf("old switch setting = %d\n",
						CWusb.GetSwitch(devID, CUSBaccess::SWITCH_0)) ;
				if (argv[1][0] == '0')
					CWusb.SetSwitch(devID, CUSBaccess::SWITCH_0, 0) ;
				else if (argv[1][0] == '1')
					CWusb.SetSwitch(devID, CUSBaccess::SWITCH_0, 1) ;
				else
					printf("Wrong argument for switches\n") ;
				break ;
			}
			else if (devType == CUSBaccess::CONTACT00_DEVICE) {
				unsigned long int mask ;
				unsigned long int value ;
				CWusb.GetMultiSwitch(devID, &mask, &value, 0) ;
				printf("Value = 0x%x\n", (int)value) ;
				}
			continue ;		// don't care about other devices
		}

		if (devType == CUSBaccess::TEMPERATURE_DEVICE || devType == CUSBaccess::TEMPERATURE2_DEVICE) {
			CWusb.ResetDevice(devID) ;
			usleep(300*1000) ;		// etwas warten

			// get 10 values
			for (int cnt=0 ; cnt < 10 ; cnt++) {
				double temperatur ;
				int	   zeit ;
				if (!CWusb.GetTemperature(devID, &temperatur, &zeit)) {
					printf("GetTemperature(%d) failed\n", devID) ;
					break ;
				}
				printf("Measured %lf Celsius, time = %d\n", temperatur, zeit) ;
				usleep(1200 * 1000) ;
			}
		}
		if (devType == CUSBaccess::HUMIDITY1_DEVICE) {
			CWusb.ResetDevice(devID) ;
			usleep(100*1000) ;		// etwas warten

			CWusb.StartDevice(devID) ;
			usleep(300*1000) ;		// etwas warten
			// get 10 values
			for (int cnt=0 ; cnt < 10 ; cnt++) {
				double temperatur, humidity ;
				int	   zeit ;
				int getOk = 0 ;
				int retry ;
				for (retry=5 ; retry > 0 && !getOk ; retry--) {
					getOk = CWusb.GetTemperature(devID, &temperatur, &zeit) ;
					if (!getOk)
						usleep(100 * 1000) ;	// retry after 100 ms
				}
				if (!getOk)
					printf("GetTemperature(%d) failed\n", devID) ;
				else {
					printf("Measured %.2lf Celsius, time = %d\n", temperatur, zeit) ;
					for (getOk=0, retry=5 ; retry > 0 && !getOk ; retry--) {
						getOk = CWusb.GetHumidity(devID, &humidity, &zeit) ;
						if (!getOk)
							usleep(100 * 1000) ;	// retry after 100 ms
					}
					if (!getOk)
						printf("GetHumidity(%d) failed\n", devID) ;
					else
						printf("Measured %.2lf %% RH, time = %d\n", humidity, zeit) ;
				}
				if (!getOk)
					CWusb.StartDevice(devID) ;	// restart device
				usleep(2200 * 1000) ;
			}
		}
	}

	CWusb.CloseCleware() ;

	return 0;
}

