// IdeTecConf [-r0/1/2] [-w t1 t2 t3 t4 t5 t6 t7 t8 t9 t10 t11 t12 PWM1start PWM1delta PWM2start PWM2delta waittime recordDelta]
// debug: -w 24 21 27 25 43 39 11 14 8 0 50 58 51 10 51 14 3 300
//
// 14.11.2013	1.0		Erste Implementierung
// xx.11.2014	1.2		Support EEROM report
// 30.01.2015	1.3		Support more than one Cleware USB device, add PWM signal
// 05.07.2016	1.4		KnoxBox V10 Controller, Log start @200, 100-1ff are humi-Temperature conversion table
// 23.02.2017	1.5		KnoxBox V12 Controller
//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "USBaccess.h"


struct ideData_struct {
	int fan1on, fan1off ;
	int fan2on, fan2off ;
	int fan3on, fan3off ;
	int heaton, heatoff ;
	int pc1on, pc1off ;
	int pc2on, pc2off ;
	int L1FanStartPWM, L1FanDT	;	// PWM range 0 - 255, 20%=51, DT increment PWN/°C
	int L2FanStartPWM, L2FanDT	;	// PWM range 0 - 255, 20%=51, DT increment PWN/°C
	int delay ;
	int recordDelta ;		// delta in seconds (0 - 0xffff)
	}  ;

enum {	IDE_HELP=0,
		IDE_READ=1,
		IDE_WRITE=2
		} action ;

void showKnoxBox(int showLog) ;		// 0=no, 1=yes, 2=continous

void write2controller(struct ideData_struct *d) ;

const char IDETECversion[] = "1.5" ;

int 
main(int argc, char *argv[]) {
	action = IDE_HELP ;
	struct ideData_struct ideData ;
	int showLog = 0 ;

	float t ;
	int sht ;

#ifdef NDEBUG
	if ((argv[0][0] | 0x20) != 'k') {
		fprintf(stderr, "%s is not a valid name\n", argv[0]) ;
		exit(1) ;
		}
#endif

	if (argc > 1) {
		if (argv[1][0] == '-' && (argv[1][1] | 0x20) == 'r') {
			action = IDE_READ ;
			if (argv[1][2] >= '0' && argv[1][2] <= '9')
				showLog = argv[1][2] - '0' ;
			}
		else if (argv[1][0] == '-' && (argv[1][1] | 0x20) == 'w') {
			while (1) {			// just to have a secure exit
				if (argc != 20) {	
					fprintf(stderr, "Argument count incorrect\n") ;
					break ;
					}
				// check args
				if (sscanf(argv[2], "%d", &ideData.fan1on) != 1 || ideData.fan1on < 0) {
					fprintf(stderr, "Fan 1 on incorrect\n") ;
					break ;
					}
				if (sscanf(argv[3], "%d", &ideData.fan1off) != 1 || ideData.fan1off < 0) {
					fprintf(stderr, "Fan 1 off incorrect\n") ;
					break ;
					}
				if (ideData.fan1on <= ideData.fan1off) {
					fprintf(stderr, "Error: Fan 1 on < off\n") ;
					break ;
					}
				if (sscanf(argv[4], "%d", &ideData.fan2on) != 1 || ideData.fan2on < 0) {
					fprintf(stderr, "Fan 2 on incorrect\n") ;
					break ;
					}
				if (sscanf(argv[5], "%d", &ideData.fan2off) != 1 || ideData.fan2off < 0) {
					fprintf(stderr, "Fan 2 off incorrect\n") ;
					break ;
					}
				if (ideData.fan2on <= ideData.fan2off) {
					fprintf(stderr, "Error: Fan 2 on < off\n") ;
					break ;
					}
				if (sscanf(argv[6], "%d", &ideData.fan3on) != 1 || ideData.fan3on < 0) {
					fprintf(stderr, "Fan 3 on incorrect\n") ;
					break ;
					}
				if (sscanf(argv[7], "%d", &ideData.fan3off) != 1 || ideData.fan3off < 0) {
					fprintf(stderr, "Fan 3 off incorrect\n") ;
					break ;
					}
				if (ideData.fan3on <= ideData.fan3off) {
					fprintf(stderr, "Error: Fan 3 on < off\n") ;
					break ;
					}
				if (sscanf(argv[8], "%d", &ideData.heaton) != 1 || ideData.heaton < 0) {
					fprintf(stderr, "Heat on incorrect\n") ;
					break ;
					}
				if (sscanf(argv[9], "%d", &ideData.heatoff) != 1 || ideData.heatoff < 0) {
					fprintf(stderr, "Heat off incorrect\n") ;
					break ;
					}
				if (ideData.heaton >= ideData.heatoff) {
					fprintf(stderr, "Error: Heat on > off\n") ;
					break ;
					}
				if (sscanf(argv[10], "%d", &ideData.pc1on) != 1 || ideData.pc1on < 0) {
					fprintf(stderr, "PC 1 on incorrect\n") ;
					break ;
					}
				if (sscanf(argv[11], "%d", &ideData.pc1off) != 1 || ideData.pc1off < 0) {
					fprintf(stderr, "PC 1 off incorrect\n") ;
					break ;
					}
				if (ideData.fan1on <= ideData.fan1off) {
					fprintf(stderr, "Error: PC 1 on < off\n") ;
					break ;
					}
				if (sscanf(argv[12], "%d", &ideData.pc2on) != 1 || ideData.pc2on < 0) {
					fprintf(stderr, "PC 2 on incorrect\n") ;
					break ;
					}
				if (sscanf(argv[13], "%d", &ideData.pc2off) != 1 || ideData.pc2off < 0) {
					fprintf(stderr, "PC 2 off incorrect\n") ;
					break ;
					}
				if (ideData.pc2on >= ideData.pc2off) {
					fprintf(stderr, "Error: PC 2 on > off\n") ;
					break ;
					}
				if (sscanf(argv[14], "%d", &ideData.L1FanStartPWM) != 1 || ideData.L1FanStartPWM < 0) {
					fprintf(stderr, "Fan 1 on incorrect\n") ;
					break ;
					}
				if (sscanf(argv[15], "%d", &ideData.L1FanDT) != 1 || ideData.L1FanDT < 0) {
					fprintf(stderr, "Fan 1 DT incorrect\n") ;
					break ;
					}
				if (sscanf(argv[16], "%d", &ideData.L2FanStartPWM) != 1 || ideData.L2FanStartPWM < 0) {
					fprintf(stderr, "Fan 2 incorrect\n") ;
					break ;
					}
				if (sscanf(argv[17], "%d", &ideData.L2FanDT) != 1 || ideData.L2FanDT < 0) {
					fprintf(stderr, "Fan 2 DT incorrect\n") ;
					break ;
					}
				if (sscanf(argv[18], "%d", &ideData.delay) != 1 || ideData.delay < 0) {
					fprintf(stderr, "delay incorrect\n") ;
					break ;
					}
				if (ideData.delay < 2) {
					fprintf(stderr, "delay %d too short\n", ideData.delay) ;
					break ;
					}
				if (sscanf(argv[19], "%d", &ideData.recordDelta) != 1 || ideData.recordDelta < 0) {
					fprintf(stderr, "recording interval incorrect\n") ;
					break ;
					}
				if (ideData.recordDelta < 10) {
					fprintf(stderr, "recording interval %d too short\n", ideData.recordDelta) ;
					break ;
					}
				if (ideData.recordDelta > 0xffff) {
					fprintf(stderr, "recording interval %d too long\n", ideData.recordDelta) ;
					break ;
					}
				action = IDE_WRITE ;		// Arguments seams to be ok
				break ;
				}
			}
		}

	printf("%s version %s\n", argv[0], IDETECversion) ;
	switch (action) { 
		case IDE_HELP:
			printf("USAGE: IdeTecConf -w L1on L1off L2on L2off L3on L3off HeatOn HeatOff PC1on PC1off PC2on PC2off PWM1start PWM1delta PWM2start PWM2delta  delay recordDelta\n") ;
			printf("   or\nIdeTecConf -r   // read settings\n") ;
			printf("   or\nIdeTecConf -r1   // read settings and log file\n") ;
			break ;
		case IDE_READ:
			showKnoxBox(showLog) ;
			break ;
		case IDE_WRITE:
			write2controller(&ideData) ;
			printf("New settings:\n") ;
			showKnoxBox(0) ;
			break ;
		}

	return 0 ;
	}

int
BCD2int(int bcd) {
	int rval = bcd & 0x0f ;
	rval += (bcd >> 4) * 10 ;
	return rval ;
	}

void
showController(CUSBaccess *CWusb, int devID) {
	if (CWusb->GetVersion(devID) < 0x8140) {
		fprintf(stderr, "knoxBox device version is 2014 type, please use other knoxBoxConf\n") ;
		return ;
		}

	printf("Ide-Tec Controller configuration:\nSerial # %d\nVersion = %d\n", CWusb->GetSerialNumber(devID), CWusb->GetVersion(devID)) ;
	printf("Switch temperature Fan 1 - On=%02dC, Off=%02dC, PWM: Start=%d, DT=%d\n", CWusb->IOX(devID, 0x10, -1), CWusb->IOX(devID, 0x11, -1),
											CWusb->IOX(devID, 0x20, -1), CWusb->IOX(devID, 0x21, -1)) ;
//											(int)(CWusb->IOX(devID, 0x20, -1) / 2.55), (int)(CWusb->IOX(devID, 0x21, -1) / 2.55)) ;
	printf("Switch temperature Fan 2 - On=%02dC, Off=%02dC, PWM: Start=%d, DT=%d\n", CWusb->IOX(devID, 0x12, -1), CWusb->IOX(devID, 0x13, -1),
											CWusb->IOX(devID, 0x22, -1), CWusb->IOX(devID, 0x23, -1)) ;
	printf("Switch temperature Fan 3 - On=%02dC, Off=%02dC\n", CWusb->IOX(devID, 0x14, -1), CWusb->IOX(devID, 0x15, -1)) ;
	printf("Switch temperature Heat  - On=%02dC, Off=%02dC\n", CWusb->IOX(devID, 0x16, -1), CWusb->IOX(devID, 0x17, -1)) ;
	printf("Switch temperature PC 1  - On=%02dC, Off=%02dC\n", CWusb->IOX(devID, 0x18, -1), CWusb->IOX(devID, 0x19, -1)) ;
	printf("Switch temperature PC 2  - On=%02dC, Off=%02dC\n", CWusb->IOX(devID, 0x1a, -1), CWusb->IOX(devID, 0x1b, -1)) ;
	printf("Time after cold start = %02d minutes (about)\n", CWusb->IOX(devID, 0x1c, -1) / 2) ;

	printf("0x7f=%02x, 0x80=%02x, 0x81=%02x\n", CWusb->IOX(devID, 0x7f, -1), CWusb->IOX(devID, 0x80, -1), CWusb->IOX(devID, 0x81, -1)) ;
	
	printf("Current Temperature = %f \370C\n", CWusb->GetTemperature(devID) ) ;

	if (CWusb->GetVersion(devID) >= 0x8141) {		// new 5-CY Board version with MCP79411 RTC
		unsigned char rm = CWusb->IOX(devID, 0x04, -1) ;
		printf("Time between temperature records = %d minutes\n", (rm >> 4) * 10 + rm & 0x0f) ;
		unsigned int dt =  CWusb->IOX(devID, 0x83, -4) ;
		printf("Current Time %d%d/%d%d/20%d%d ", (dt>>20)&15, (dt>>16)&15, (dt>>12)&15, (dt>>8)&15, (dt>>4)&15, dt&15) ;
		dt =  CWusb->IOX(devID, 0x80, -4) ;
		printf("%d%d:%d%d:%d%d \n", (dt>>12)&15, (dt>>8)&15, (dt>>20)&15, (dt>>16)&15, (dt>>28)&7, (dt>>24)&15) ;
		}
	else {
		printf("Time between temperature records = %d seconds\n", CWusb->IOX(devID, 0x04, -1) + 256 * CWusb->IOX(devID, 0x05, -1)) ;
		unsigned int dt =  CWusb->IOX(devID, 0x43, -4) ;
		printf("Current Time %d%d/%d%d/20%d%d ", (dt>>28)&15, (dt>>24)&15, (dt>>12)&15, (dt>>8)&15, (dt>>4)&15, dt&15) ;
		dt =  CWusb->IOX(devID, 0x40, -4) ;
		printf("%d%d:%d%d:%d%d \n", (dt>>12)&15, (dt>>8)&15, (dt>>20)&15, (dt>>16)&15, (dt>>28)&15, (dt>>24)&15) ;
		}
	}

void
show1stBlock(CUSBaccess *CWusb, int devID, int start=0, int end=256) {
	unsigned int b4 = 0 ;
	for (int i = start ; i < end ; i += 4) {
		b4 = CWusb->IOX(devID, i, -4) ;
		if ((i & 0x0f) == 0) 
			printf("%02x: ", i) ;
		printf("%02x %02x %02x %02x", (b4>>24)&0xff, (b4>>16)&0xff, (b4>>8)&0xff, b4&0xff) ;
		switch (i & 0xf) {
			case 0:
			case 8:
				printf("  ") ;
				break ;
			case 4:
				printf("    ") ;
				break ;
			case 12:
				printf("\n") ;
				break ;
			}
		}
	}

void
showLogRom(CUSBaccess *CWusb, int devID) {
	printf("\n\nStart of log\n") ;
	// now read the log
	int endPage =CWusb->IOX(devID, 0x1e, -1) ;
	int endInBlock = CWusb->IOX(devID, 0x1f, -1) ;
	int EndOFEEROM = (endPage << 8) + endInBlock ;
	int start = 0x0100 ;
	while (start < EndOFEEROM && start < 0x10000) {
		unsigned char d[256] ;
		int page = start >> 8 ;
		int end = 0x100 - (start & 0xff) ;
		int i ;
		for (i=0 ; i < end ; i += 4) {
			unsigned int ld = CWusb->IOX(devID, start+i, -4) ;
			d[i+3] = ld & 0xff ;
			d[i+2] = (ld >> 8) & 0xff ;
			d[i+1] = (ld >> 16) & 0xff ;
			d[i+0] = (ld >> 24) & 0xff ;
			}
		i = 0 ;
		if (page == endPage)
			end = endInBlock - (start & 0xff) ;
		while (i < end) {
			// printf("0x%02x%02x: ", page, i) ;
			switch (d[i]) {
				case 0x80:			// Power on
					printf("\n%s\nPower on @ %02d/%02d/20%02d %02d:%02d:%02d\n",
						"===================================================",
						BCD2int(d[i+4]), BCD2int(d[i+5]), BCD2int(d[i+6]), 
						BCD2int(d[i+3]), BCD2int(d[i+2]), BCD2int(d[i+1])) ;
					i += 7 ;
					break ;
				case 0x81:	{		// switch change
					double temperature = d[i+4] ;
					printf("\nSwitch state @ %02d:%02d:%02d %3.1lf \370C ", 
						BCD2int(d[i+3]), BCD2int(d[i+2]), BCD2int(d[i+1]), temperature) ;
					unsigned int r = d[i+5] ;
					printf("PC=%d, MO=%d, H=%d, F1=%d, F2=%d, F3=%d, Fx2=%d, Fx3=%d",
						r&0x01, (r&0x02) ? 1 : 0, (r&0x04) ? 1 : 0, (r&0x08) ? 1 : 0,
						(r&0x10) ? 1 : 0, (r&0x20) ? 1 : 0, (r&0x40) ? 1 : 0, (r&0x80) ? 1 : 0) ;
					if (CWusb->GetVersion(devID) >= 0x8140) {
						printf(", Input = 0x%02x", d[i+6]) ;
						i += 7 ;
						} 
					else
						i += 6 ;
					printf("\n") ;
					break ;
					}
				case 0x82:			// Next day
					printf("\nNext Day @ %02d/%02d/20%02d, ", BCD2int(d[i+1]), BCD2int(d[i+2]), BCD2int(d[i+3])) ;
					i += 4 ;
					break ;
				case 0x83:			// last block finished, open new block
					printf("\nNew block %04x @ %02d/%02d/20%02d %02d:%02d:%02d, ", (start & 0xff00) + i,
						BCD2int(d[i+4]), BCD2int(d[i+5]), BCD2int(d[i+6]), 
						BCD2int(d[i+3]), BCD2int(d[i+2]), BCD2int(d[i+1])) ;
					i += 7 ;
					break ;
				case 0x84:			// time changed
					printf("\nTime update @ %02d/%02d/20%02d %02d:%02d:%02d\n",
						BCD2int(d[i+4]), BCD2int(d[i+5]), BCD2int(d[i+6]), 
						BCD2int(d[i+3]), BCD2int(d[i+2]), BCD2int(d[i+1])) ;
					i += 7 ;
					break ;
				case 0x86:			// end of block
					printf("End of Block found\n") ;
					if (i < 0x80)
						i = 0x80  - (start & 0xff) ;
					else
						end = i ;
					break ;
				case 0xff:	{		// negative temperatur
					int value = (d[i+2] << 5) + (d[i+1] >> 3) ;
					if (value & 0x1000)		// negativ!
						value = (value & 0xfff) - 0x1000 ;
					double temperature = value * 0.0625 ;
					printf("-Temp %3.1lf\370C\n",  temperature) ;
					i += 3 ;
					break ;
					}
				default: {			// this should be a temperature
					double temperature = d[i] ;
					printf("%3.1lf\370C, ",  temperature) ;
					i += 1 ;
					break ;
					}
				}
			}
		start = (start & 0xff00) + 0x100;		// next page
		}
	}

void
showKnoxBox(int showLog) {
		
		
	CUSBaccess *CWusb = 0 ;
	int knoxID = -1 ;
	
	CWusb = new CUSBaccess ;
	if (CWusb == NULL) {
		fprintf(stderr, "Can't open USBaccess!\n") ;
		return ;
		}
	int USBcount = CWusb->OpenCleware() ;
	printf("found %d Cleware devices\n", USBcount) ;

	for (int devID=0 ; devID < USBcount ; devID++) {
		int knoxBoxType = ( CWusb->IsIdeTec(devID) & 0x0f) ;
		printf("\ndevID %d, knoxBox=%d\n", devID, knoxBoxType) ;
		switch (knoxBoxType) {
			default:
			case 0:
				printf("type %d, version %d is not a knoxBox\n", CWusb->GetUSBType(devID), CWusb->GetVersion(devID)) ;
				continue ;		// not a knoxBox
			case 1:
			case 3:
				printf("\n -------------  Master Controller -------------------\n") ;
				knoxID = devID ;
				showController(CWusb, devID) ;
				show1stBlock(CWusb, devID) ;
				break ;
			case 2:
				printf("\n -------------  Relais Controller -------------------\n") ;
				show1stBlock(CWusb, devID, 0x40, 0x70) ;
				break ;
			case 4:
				printf("\n -------------  Sensor Controller -------------------\n") ;
				show1stBlock(CWusb, devID, 0x40, 0x70) ;
				break ;
			case 5:
				printf("\n -------------  Tacho Controller -------------------\n") ;
				show1stBlock(CWusb, devID, 0x40, 0x70) ;
				break ;
			}

		}

	if (showLog != 0 && knoxID >= 0) 
		showLogRom(CWusb, knoxID) ;

	CWusb->CloseCleware() ;
	delete CWusb ;
	}


void 
write2controller(struct ideData_struct *d) {
#if 0
	CUSBaccess *CWusb = 0 ;
	int devID = -1 ;
	
	CWusb = new CUSBaccess ;
	if (CWusb == NULL) {
		fprintf(stderr, "Can't open USBaccess!\n") ;
		return ;
		}
	int USBcount = CWusb->OpenCleware() ;
	for (devID=0 ; devID < USBcount ; devID++) {
		if (CWusb->IsIdeTec(devID))
			break ;
		}
	if (USBcount <= devID) { // we accept just one IDETec device
		fprintf(stderr, "we found no IDETec device\n") ;
		return ;
		}


	if (CWusb->GetVersion(devID) < 0x8140) {
		fprintf(stderr, "IDETec device version is 2014 type, please use other IdeTecConf\n") ;
		return ;
		}

/*	if (CWusb->GetVersion(devID) >= 0x8141) {			// we need table to convert humi values
		fprintf(stderr, "start Humidity sensor table programming: \n") ;
		for (int i = 0; i <= 255; i++) {
			CWusb->IOX(devID, 0x100+i, SHTconfTable[i]&0xff) ;
			fprintf(stderr, "%03d\r", 255-i) ;
			}
		}
*/

	fprintf(stderr, "Start programming ") ;
	// setup some values anyway
	CWusb->IOX(devID, 0, 0) ; fprintf(stderr, ".") ;		// clear 7seg state, otherwise the display is empty
	CWusb->IOX(devID, 6, 0) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 7, 0) ; fprintf(stderr, ".") ;		// Contact configuration
	CWusb->IOX(devID, 3, 12) ; fprintf(stderr, ".") ;		// 12 Contacts

	CWusb->IOX(devID, 0x10, d->fan1on) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x11, d->fan1off) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x12, d->fan2on) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x13, d->fan2off) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x14, d->fan3on) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x15, d->fan3off) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x16, d->heaton) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x17, d->heatoff) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x18, d->pc1on) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x19, d->pc1off) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x1a, d->pc2on) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x1b, d->pc2off) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x1c, d->delay*2) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x20, d->L1FanStartPWM) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x21, d->L1FanDT) ; fprintf(stderr, "\n") ;
	CWusb->IOX(devID, 0x22, d->L2FanStartPWM) ; fprintf(stderr, ".") ;
	CWusb->IOX(devID, 0x23, d->L2FanDT) ; fprintf(stderr, "\n") ;

	time_t osBinaryTime;  // C run-time time (defined in <time.h>)
	time(&osBinaryTime) ;  // Get the current time from the operating system.
	CTime time1(osBinaryTime);  // CTime from C run-time time
	int y = time1.GetYear() - 2000 ;
	int bcd ;
	if (CWusb->GetVersion(devID) >= 0x8141) {		// new 5-CY Board version with MCP79411 RTC
		int delta = d->recordDelta / 60 ;
		if (delta <= 0)
			delta = 1 ;
		else if (delta > 30)
			delta = 30 ;
		bcd = (delta % 10) + (delta / 10) * 16 ;			// new controller need the distance in minutes in BCD format
		CWusb->IOX(devID, 0x04, delta) ; fprintf(stderr, "\n") ;
		CWusb->IOX(devID, 0x05, 0) ; fprintf(stderr, "\n") ;

		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x86, bcd) ; fprintf(stderr, ".") ;
		y = time1.GetMonth()  ;
		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x85, bcd) ; fprintf(stderr, ".") ;
		y = time1.GetDay()  ;
		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x84, bcd) ; fprintf(stderr, ".") ;
		bcd = 0x08 ;		// enable Battery   here is the place to store day of week, if this is needed for any purpose
		CWusb->IOX(devID, 0x83, bcd) ; fprintf(stderr, ".") ;
		y = time1.GetHour()  ;
		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x82, bcd) ; fprintf(stderr, ".") ;
		y = time1.GetMinute()  ;
		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x81, bcd) ; fprintf(stderr, ".") ;
		y = time1.GetSecond()  ;
		bcd = (y % 10) + (y / 10) * 16 + 0x80 ;		// BCD seconds + enable Oscillator bit
		CWusb->IOX(devID, 0x80, bcd) ; fprintf(stderr, ".") ;
		CWusb->IOX(devID, 0x87, 0x30) ; fprintf(stderr, ".") ;		// disable external oscilator, enable Alarm 1+2
		}
	else {
		CWusb->IOX(devID, 0x04, d->recordDelta & 0xff) ; fprintf(stderr, "\n") ;
		CWusb->IOX(devID, 0x05, (d->recordDelta >> 8) & 0xff) ; fprintf(stderr, "\n") ;

		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x46, bcd) ; fprintf(stderr, ".") ;
		y = time1.GetMonth()  ;
		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x45, bcd) ; fprintf(stderr, ".") ;
		y = time1.GetDay()  ;
		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x43, bcd) ; fprintf(stderr, ".") ;
		y = time1.GetHour()  ;
		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x42, bcd) ; fprintf(stderr, ".") ;
		y = time1.GetMinute()  ;
		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x41, bcd) ; fprintf(stderr, ".") ;
		y = time1.GetSecond()  ;
		bcd = (y % 10) + (y / 10) * 16 ;
		CWusb->IOX(devID, 0x40, bcd) ; fprintf(stderr, ".") ;
		}

	CWusb->IOX(devID, 0, 0xff) ; fprintf(stderr, "\n") ;		// start record here

//	CWusb->IOX(devID, 0x40, 0x87) ; fprintf(stderr, "\n") ;		// this cause a initialisazion of the EEROM
//	CWusb->IOX(devID, 0x41, 0x87) ; fprintf(stderr, "\n") ;

	CWusb->CloseCleware() ;
	delete CWusb ;
#endif
	}
