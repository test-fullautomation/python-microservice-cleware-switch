/* USBswitchCmd [-n device] [0 | 1] [R | Y | G | O] [-d]
 *           -n device   use device with this serial number
 *						 device may also be the Regsitry base, e.g.08-17-0000d986-
 *           0 | 1       turns switch off(0) or on(1)
 *           R Y G O	 turns switch Red, Yellow, Green or Off light, used for traffic lights
 *           -d          print debug infos
 *           -s          secure switching - wait and ask if switching was done
 *           -r          read the current setting
 *           -t          reseT the device
 *           -# switch#  select switch for multiple switch device, first=0, multiple comma seperated
 *           -i nnn      interval test, turn endless on/off and wait nnn ms between state change
 *           -I nnn      interval test, turn once on/off and wait nnn ms between state change
 *			 -p t1 .. tn pulse mode, turn the switch 0.5 s on, then off wait t1 seconds, turn on,off wait t2 seconds etc.
 *			 -b			 binary mode, set or read all channels together, channel binary coded (1,2,4,8,16,....)
 *           -v          print version
 *           -h          print command usage
 *
 * (C) Copyright 2003-2016 Cleware GmbH
 *
 * Version     Date     Comment
 *   1.0    03.04.2003	Initial coding
 *  3.6.0   20.11.2007	Do manipulation only in the registry - ClewareService will do the rest
 *  3.6.1   13.06.2008	Implement interval test to evaluate how fast the switch may turn
 *  3.6.2   18.01.2010	Implement pulse mode
 *  3.6.3   04.02.2010	Extend Registry mode, Implement single Interval switch
 *  4.0.0   03.05.2010	Moved to VC2008
 *	4.0.1	18.03.2011	Added Ampel codes RYGO
 *	4.0.2	28.06.2011	Allow Watchdog switching
 *	4.0.3	04.07.2013	list option
 *	4.0.4	14.04.2014	avoid multiple switch sending, only if state != newstate
 *  4.0.5	10.09.2014	for compatibility reason support SwitchboxUSB  <sernum> state
 *  4.0.6	09.02.2015	allow multiple switches
 *  4.0.7	09.03.2015	introduce binary mode
 *  4.0.8	01.02.2016	ygro options must set setState
 *  4.0.9	03.10.2016	bug fixes: blinking: is ignored when switch is on, allow multiple siwtchID
 *  4.0.10	03.03.2017	 Y -p 1 1 1  pulse should take color in mind
 *  5.0.0	29.09.2017	 aded -R option to just return the state without print
 *
 */

char *versionString = "5.0.0" ;
char *progName ;

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "USBaccess.h"

const int MAXSWITCH = 16 ;

enum tl_enum { TL_none=1, TL_red=2, TL_green=4, TL_yellow=8 } ;

int GetSwitch(int devID, enum CUSBaccess::SWITCH_IDs switchID, CUSBaccess *CWusb) ;
int SetSwitch(int devID, enum CUSBaccess::SWITCH_IDs switchID, int turnSwitch, CUSBaccess *CWusb) ;

int doSwitch(int debug, int doRead, int printRead, int secureSwitching, 
			  int turnSwitch, int serialNumber, int traffic, int binaryMode,
			  int resetDevice, int intervalTestMS, int intervalCount,
			  char **pulsieren, int switchID[]) ;

void
Sleep(int ms) {	// interface to Windows function
	usleep(ms*1000) ; // microseconds
	}

main(int argc, char* argv[]) {
	int switchID[MAXSWITCH] =  { CUSBaccess::SWITCH_0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }  ;
	int debug = 0 ;
	int state = -1 ;		// 0=off, 1=on
	int doRead = 0 ;
	int secureSwitching = 0 ;
	int binaryMode = 0 ;
	int turnSwitch = -1 ;	// 0=off, 1=on, bbbbbbbbb in case of binary mode
	int printVersion = 0 ;
	int printHelp = 0 ;
	int serialNumber = -1 ;
	int resetDevice = 0 ;
	int listDevices = 0 ;
	char **pulsieren = 0 ;
	int intervalTestMS = 0 ;
	int intervalCount = -1 ;		// -1= endless
	int printRead = 0 ;
	int traffic = TL_none ;
	int ok = 1 ;

	progName = argv[0] ;

	if (strcmp(argv[0], "SwitchboxUSB") == 0) {
		int OnOffPos = 2 ;
		if (sscanf(argv[1], "%d", &serialNumber) != 1) {
			OnOffPos = 1 ;
			}
		if (strcmp(argv[OnOffPos], "ON") == 0)
			turnSwitch = 1 ;
		else if (strcmp(argv[OnOffPos], "OFF") == 0)
			turnSwitch = 0 ;
		else 
			ok = 0 ;
		}
	else {
		for (argc--, argv++ ; argc > 0 && ok ; argc--, argv++) {
			switch (argv[0][0]) {
				case 'r':
				case 'R':
					traffic &= ~TL_none ;
					traffic |= TL_red ;
					turnSwitch = 0 ;		// otherwise doSwitch takes this as a read command
					break ;
				case 'y':
				case 'Y':
					traffic &= ~TL_none ;
					traffic |= TL_yellow ;
					turnSwitch = 0 ;		// otherwise doSwitch takes this as a read command
					break ;
				case 'g':
				case 'G':
					traffic &= ~TL_none ;
					traffic |= TL_green ;
					turnSwitch = 0 ;		// otherwise doSwitch takes this as a read command
					break ;
				case 'o':
				case 'O':
					traffic &= ~TL_none ;
					turnSwitch = 0 ;		// otherwise doSwitch takes this as a read command
					break ;
				case '-':
					switch (argv[0][1]) {
						case 'd':
						case 'D':
							debug = 1 ;
							break ;
						case 's':
						case 'S':
							secureSwitching = 1 ;
							break ;
						case 't':
						case 'T':
							resetDevice = 1 ;
							break ;
						case 'r':
							printRead = 1 ;
						case 'R':
							doRead = 1 ;
							break ;
						case 'v':
						case 'V':
							printVersion = 1 ;
							break ;
						case 'l':
						case 'L':
							listDevices = 1 ;
							break ;
						case 'b':
						case 'B':
							binaryMode = 1 ;
							break ;
						case 'h':
						case 'H':
							printHelp = 1 ;
							break ;
						case 'P':
						case 'p': {
							if (argc == 1) {
								printf("missing pulse delay %s\n", *argv) ;
								ok = 0 ;
								break ;
								}
							argc-- ;
							argv++ ;
							pulsieren = argv ;
							int sw = 0 ;
							while (*argv != NULL && sscanf(*argv, "%d", &sw) == 1 && sw >= 1) {
								argv++ ;
								argc-- ;
								}
							break ;
							}
						case 'I':
							intervalCount = 1 ;
						case 'i': {
							if (argc == 1) {
								printf("missing interval %s\n", *argv) ;
								ok = 0 ;
								break ;
								}
							argc-- ;
							argv++ ;
							turnSwitch = 1 ;		// start with ON
							if (sscanf(*argv, "%d", &intervalTestMS) != 1 || intervalTestMS < 1) {
								printf("illegal interval %s\n", *argv) ;
								ok = 0 ;
								break ;
								}
							break ;
							}
						case '#': {
							if (argc == 1) {
								printf("missing switch %s\n", *argv) ;
								ok = 0 ;
								break ;
								}
							argc-- ;
							argv++ ;
							int sw = 0 ;
							int i = 0 ;
							for (char *pt=*argv  ; *pt != '\0' && i < MAXSWITCH ; i++) {
								if (*pt < '0' || *pt > '9') {
									printf("illegal switch number %s\n", *argv) ;
									ok = 0 ;
									break ;
									}
								sw = *pt++ - '0' ;
								if (*pt >= '0' && *pt <= '9')
									sw = sw*10 + *pt++ - '0' ;
								switchID[i] = sw + CUSBaccess::SWITCH_0 ;
								if (*pt == ',')
									pt++ ;
								else 
									break ;			// bullshit
								}
							break ;
							}
						case 'n':
						case 'N':
							if (argc == 1) {
								printf("missing serial number %s\n", *argv) ;
								ok = 0 ;
								break ;
								}
							argc-- ;
							argv++ ;
							if (sscanf(*argv, "%d", &serialNumber) != 1) {
								printf("illegal serial number %s\n", *argv) ;
								ok = 0 ;
								break ;
								}
							break ;
						default:
							printf("illegal argument %s\n", *argv) ;
							ok = 0 ;
							break ;
						}
					break ;
				default: {
					char *pt = *argv ;
					if (*pt < '0' || *pt > '9') {
						printf("illegal argument %s\n", *argv) ;
						ok = 0 ;
						break ;
						}
					for (turnSwitch=0 ; *pt != '\0' ; pt++) {
						if (*pt < '0' || *pt > '9')
							break ;
						turnSwitch = (turnSwitch * 10) + *pt - '0' ;
						}
					}
				}
			}
		}

	if (!ok)
		return -1 ;

	if (printHelp) {
		printf("Usage: %s [-n device] [0 | 1] [-...]\n", progName) ;
		printf("       -n device   use device with this serial number or registry base\n") ;
		printf("       0 | 1       turns switch off(0) or on(1)\n") ;
		printf("   r | y | g | o   turns red, yellow, green on or all off\n") ;
		printf("       -r          read the current setting\n") ;
		printf("       -R          return the current setting\n") ;
		printf("       -# switch#  select switch for multiple switch device, first=0, multiple comma seperated\n") ;
		printf("       -b			 binary mode, set or read all channels together, channel binary coded (1,2,4,8,16,....)\n") ;
		printf("       -i nnn      interval test, turn endless on/off and wait nnn ms between state change\n") ;
		printf("       -I nnn      interval test, turn once on/off and wait nnn ms between state change\n") ;
		printf("       -p t1 .. tn pulse, turn the switch 0.5 sec. on, then off wait t1 sec., turn on,off wait t2 sec.etc\n") ;
		printf("       -s          secure switching - wait and ask if switching was done\n") ;
		printf("       -v          print version\n") ;
 		printf("       -h          print command usage\n") ;
		printf("       -l          list devices\n") ;
		printf("       -t          reseT the device\n") ;
		printf("       -d          print debug infos\n") ;
		}

	if (printVersion) {
		printf("%s version %s\n", progName, versionString) ;
		}
	else if (listDevices) {
		CUSBaccess *CWusb = new CUSBaccess ;
		if (CWusb != NULL) {
			int USBcount = CWusb->OpenCleware() ;
			int devID ;
			for (devID=0 ; devID < USBcount ; devID++)
				printf("Device %d: Type=%d, Version=%d, SerNum=%d\n", devID,
									CWusb->GetUSBType(devID), CWusb->GetVersion(devID),
									CWusb->GetSerialNumber(devID)) ;
			}
		}
	else {
		state = doSwitch(	debug, doRead, printRead, secureSwitching, turnSwitch, serialNumber, traffic, binaryMode,
							resetDevice, intervalTestMS, intervalCount, pulsieren, switchID) ;
		}
	
	exit(state) ;
	}


int 
doSwitch( int debug, 
		  int doRead, 
		  int printRead, 
		  int secureSwitching, 
		  int turnSwitch, 
		  int serialNumber, 
		  int traffic,
		  int binaryMode,
		  int resetDevice,
		  int intervalTestMS,
		  int intervalCount,		// -1 == endless
		  char **pulsieren,
		  int switchID[]
		  ) {
	CUSBaccess *CWusb = 0 ;
	int ok = 1 ;
	int USBcount = 1;
	int state = -1 ; 
	
	CWusb = new CUSBaccess ;
	if (CWusb == NULL) {
		fprintf(stderr, "Can't open USBaccess!\n") ;
		ok = 0 ;
		}
	else {
		USBcount = CWusb->OpenCleware() ;

		if (debug) {
			printf("%s version %s, USBaccess version %d, expected %d\n", progName, versionString, CWusb->GetDLLVersion(), USBaccessVersion ) ;
			printf("OpenCleware found %d devices\n", USBcount) ;
			}
		}

	int devID ;
	for (devID=0 ; ok && devID < USBcount ; devID++) {
		if (CWusb != NULL) {
			if (debug) {
				printf("Device %d: Type=%d, Version=%d, SerNum=%d\n", devID,
							CWusb->GetUSBType(devID), CWusb->GetVersion(devID),
							CWusb->GetSerialNumber(devID)) ;
				}
			int devType = CWusb->GetUSBType(devID) ;
			if (	devType != CUSBaccess::SWITCH1_DEVICE 
				&&	devType != CUSBaccess::CONTACT00_DEVICE
				&&	devType != CUSBaccess::SWITCHX_DEVICE
				&&	devType != CUSBaccess::WATCHDOGXP_DEVICE
				&&	devType != CUSBaccess::WATCHDOG_DEVICE
				)
				continue ;
			if (serialNumber > 0  && CWusb->GetSerialNumber(devID) != serialNumber)
				continue ;
			}
		if (debug) {
			int switchCnt, buttonCnt ;
			CWusb->GetSwitchConfig(devID, &switchCnt, &buttonCnt) ;
			printf("switch config got %d switches, %d buttons\n", switchCnt, buttonCnt) ;
			}
		if (binaryMode && CWusb != NULL) {
			unsigned long int ms = 0 ;
			if (CWusb->GetUSBType(devID) != CUSBaccess::CONTACT00_DEVICE) {
				printf("-b not supported by this device\n") ;
				break ;
				}
			if (CWusb->GetMultiSwitch(devID, 0, &ms, 0) == -1)
				state = -1 ;
			else
				state = ms ;
			}
		else
			state = GetSwitch(devID, (enum CUSBaccess::SWITCH_IDs) switchID[0], CWusb) ;
		if (debug)
			printf("old switch setting = %d\n", state) ;
		if (traffic == TL_none && state == turnSwitch && switchID[1] == 0 && intervalTestMS == 0)		// only check when one switch is selected
			return state ;		// early version of multi2 toggles when turned on

		state = turnSwitch ;

		int switch2turn = switchID[0] ;
		if (traffic & TL_red)
			switch2turn = CUSBaccess::SWITCH_0 ;
		if (traffic & TL_green)
			switch2turn = CUSBaccess::SWITCH_2 ;
		if (traffic & TL_yellow)
			switch2turn = CUSBaccess::SWITCH_1 ;
		for (int shot=2 ; ok && pulsieren != NULL ; shot++) {
			if (!SetSwitch(devID,(enum CUSBaccess::SWITCH_IDs) switch2turn, 1, CWusb)) {
				ok = 0 ;
				break ;
				}
			Sleep(500) ;	
			if (!SetSwitch(devID, (enum CUSBaccess::SWITCH_IDs)switch2turn, 0, CWusb)) {
				ok = 0 ;
				break ;
				}
			int delay = 0 ;
			state = 0 ;
			if (*pulsieren == NULL || sscanf(*pulsieren, "%d", &delay) != 1 || delay < 1)
				break ;		// all done
			while (delay > 0) {
				if (debug)
					printf("\rShot %d in %d sec.   ", shot, delay) ;
				Sleep(1000) ;		// ms
				delay-- ;
				}
			if (debug)
				printf("\rs h o t ******                ") ;
			pulsieren++ ;
			}

		while (ok && intervalTestMS > 0) {
//			ok = SetSwitch(devID, (enum CUSBaccess::SWITCH_IDs)switchID[0], turnSwitch, CWusb) ;
			for (int i = 0; i < MAXSWITCH; i++) {
				if (switchID[i] == 0)
					break;
				ok = SetSwitch(devID, (enum CUSBaccess::SWITCH_IDs)switchID[i], turnSwitch, CWusb);
				}

			if (!ok) {
				printf("USBswitch cannot be reached\n") ;
				ok = 0 ;
				break ;
				}	
			if (debug)
				printf("%d", state) ;
			Sleep(intervalTestMS) ;	
			if (turnSwitch)
				turnSwitch = 0 ;
			else
				turnSwitch = 1 ;
			if (intervalCount > 0)
				intervalCount-- ;
			else if (intervalCount == 0) {
				state = turnSwitch ;
				turnSwitch = -1 ;
				break ;
				}
			}
		
		if (turnSwitch != -1) { 			// we just have to read the device
			if (traffic == TL_none) {
				if (binaryMode && CWusb != NULL) {
					ok = CWusb->SetMultiSwitch(devID, turnSwitch)  ;
					ok =  (ok != -1) ;			// SetSwitch uses different failed code
					}
				else {
					for (int i=0 ; i < MAXSWITCH ; i++) {
						if (switchID[i] == 0)
							break ;
//						if (turnSwitch == 0)
//							ok = SetSwitch(devID, (enum CUSBaccess::SWITCH_IDs)switchID[i], 0, CWusb) ;
//						else if (turnSwitch == 1)
//							ok = SetSwitch(devID, (enum CUSBaccess::SWITCH_IDs)switchID[i], 1, CWusb) ;
						ok = SetSwitch(devID, (enum CUSBaccess::SWITCH_IDs)switchID[i], turnSwitch, CWusb) ;		// use turnswitch directly
						}
					}
				}
			else {
				if (traffic & TL_red) 
					ok = SetSwitch(devID, CUSBaccess::SWITCH_0, 1, CWusb) ;
				else
					ok = SetSwitch(devID, CUSBaccess::SWITCH_0, 0, CWusb) ;
				if (traffic & TL_green)
					ok = SetSwitch(devID, CUSBaccess::SWITCH_2, 1, CWusb) ;
				else
					ok = SetSwitch(devID, CUSBaccess::SWITCH_2, 0, CWusb) ;
				if (traffic & TL_yellow)
					ok = SetSwitch(devID, CUSBaccess::SWITCH_1, 1, CWusb) ;
				else
					ok = SetSwitch(devID, CUSBaccess::SWITCH_1, 0, CWusb) ;
				}
			if (ok == 0) {
				printf("USBswitch cannot be reached\n") ;
				state = -1 ;
				break ;
				}
			}
		if (secureSwitching && turnSwitch >= 0) {
			for (int tryCnt=0 ; tryCnt < 5 ; tryCnt++) {
				Sleep(500) ;
				state = GetSwitch(devID, (enum CUSBaccess::SWITCH_IDs)switchID[0], CWusb) ;
				if (turnSwitch == state)
					break ;
				if (debug)
					printf("retry switch!\n") ;
				SetSwitch(devID, (enum CUSBaccess::SWITCH_IDs)switchID[0], turnSwitch, CWusb) ;
				}
			}
		if (doRead) {
			if (binaryMode && CWusb != NULL) {
				unsigned long int ms = 0 ;
				if (CWusb->GetMultiSwitch(devID, 0, &ms, 0) == -1)
					state = -1 ;
				else
					state = ms ;
				if (debug)
					printf("state = 0x%04x  -  ", ms) ;
				if (printRead)
					printf("%d", state) ;
				}
			else {
				state = GetSwitch(devID, (enum CUSBaccess::SWITCH_IDs)switchID[0], CWusb) ;
				char c = '0' ;
				if (state == 1)
					c++ ;
				if (printRead)
					printf("%c\n", c) ;
				}
			}
		if (resetDevice && CWusb != NULL)
			CWusb->ResetDevice(devID) ;
		break ;		// only one switch supported now
		}

	if (devID >= USBcount)
		printf("USBswitch not found\n") ;

	if (CWusb != NULL) {
		CWusb->CloseCleware() ;
		delete CWusb ;
		}

	if (!ok)
		state = -1 ;

	return state ;
	}

int
GetSwitch(int devID, enum CUSBaccess::SWITCH_IDs switchID, CUSBaccess *CWusb) {
	int rval = -1 ;		// -1 =error, 0,1 if successful
	if (CWusb != NULL)
		rval = CWusb->GetSwitch(devID, switchID) ;
	return rval ;
	}

int
SetSwitch(int devID, enum CUSBaccess::SWITCH_IDs switchID, int turnSwitch, CUSBaccess *CWusb) {
	int ok = 0 ;		// 1 if successful
	if (CWusb != NULL)
		ok = CWusb->SetSwitch(devID, switchID, turnSwitch) ;
	return ok ;
	}

