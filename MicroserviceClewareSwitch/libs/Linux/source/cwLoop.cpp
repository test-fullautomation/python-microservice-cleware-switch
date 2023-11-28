// cwLoop.cpp :
// loop for sending values to clewarecontrol
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

#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include "string.h" 
#include "send2cc.h" 
#include "USBaccess.h"

#include "errno.h"

#define Sleep(ms) usleep(ms * 1000)

const int maxClewareDevices = 16 ;
volatile int cwSignal = 0 ;

int 
cwLoop(char *serverName, int port, int debugEnabled) {
	CUSBaccess CWusb ;

	int USBcount = CWusb.OpenCleware() ;
	if (debugEnabled)
		printf("OpenCleware found %d devices\n", USBcount) ;

	if (USBcount <= 0) {
		printf("no Cleware devices found!\n") ;
		return 0 ;
		}

	if (USBcount > maxClewareDevices) {
		printf("too many Cleware devices found (%d)!\n", USBcount) ;
		return 0 ;
		}

	struct cwStruct cwInfo[maxClewareDevices] ;
	const int time2sleep = 500 ;		// ask every 500 ms 
	for (int i=0 ; i < maxClewareDevices ; i++) {
		cwInfo[i].tempTime = 0 ;
		cwInfo[i].sleepTime = 2000 ;			// wait 2 seconds
		cwInfo[i].elapsedTime = 2000 ;			// wait 2 seconds
		}


	cwSignal = 0 ;
	sighandler_t old_signal ;
	old_signal = signal(SIGPIPE, (sighandler_t) catchIOproblems) ;
	if (old_signal == SIG_ERR) {
		printf("Cannot install signal handler\n") ;
		return 0 ;
		}

	// establish the socket
	int sd, rc, i;
	struct sockaddr_in localAddr, servAddr;
	struct hostent *h;
  
	h = gethostbyname(serverName) ;
	if (h == NULL) {
		printf("%s: unknown host '%s'\n", serverName);
		return 0 ;
		}
	if (debugEnabled)
		printf("host %s found\n", serverName) ;

	servAddr.sin_family = h->h_addrtype;
	memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	servAddr.sin_port = htons(port) ;

	/* create socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0) {
		printf("cannot open socket, errno=%d\n", errno) ;
		return 0 ;
		}
	if (debugEnabled)
		printf("socket %d open\n", port) ;

	/* bind any port number */
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(0);
  
	rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
	if (rc < 0) {
		printf("cannot bind port TCP %u, errno=%d\n", port, errno);
		return 0 ;
		}
	if (debugEnabled)
		printf("port %d binded\n", port) ;
				
	/* connect to server */
	rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
	if (rc < 0) {
		printf("cannot connect, errno=%d\n", errno);
		return 0 ;
		}
	if (debugEnabled)
		printf("server %s connected\n", serverName) ;

	// setup device infos
	for (int devID=0 ; devID < USBcount ; devID++) {
		int devType = CWusb.GetUSBType(devID) ;
		cwInfo[devID].devType = devType ;
		cwInfo[devID].serialNumber = CWusb.GetSerialNumber(devID) ;
		cwInfo[devID].version = CWusb.GetVersion(devID) ;
		if (devType == CUSBaccess::TEMPERATURE_DEVICE  || devType == CUSBaccess::TEMPERATURE2_DEVICE) {
			CWusb.ResetDevice(devID) ;
			cwInfo[devID].elapsedTime += 3000 ;		// wait a bit longer
			if (debugEnabled)
				printf("found device %d, Type %d\n", devID, devType) ;
			continue ;
			}
		if (devType == CUSBaccess::HUMIDITY1_DEVICE) {
			CWusb.StartDevice(devID) ;
			cwInfo[devID].elapsedTime += 1000 ;		// wait a bit longer
			cwInfo[devID].sleepTime = 3000 ;		// wait 3 seconds
			if (debugEnabled)
				printf("found device %d, Type %d\n", devID, devType) ;
			continue ;
			}
		}

	int errorCount = 0 ;
	while (cwSignal == 0) {
		for (int devID=0 ; devID < USBcount ; devID++) {
			if (cwInfo[devID].elapsedTime > 0) {
				cwInfo[devID].elapsedTime -= time2sleep ;
				continue ;
				}
				
			cwInfo[devID].elapsedTime = cwInfo[devID].sleepTime ;		// time to next query
			switch (cwInfo[devID].devType) {
				case CUSBaccess::SWITCH1_DEVICE: {
					break ;
					}

				case CUSBaccess::TEMPERATURE2_DEVICE:
				case CUSBaccess::TEMPERATURE_DEVICE: {
					int zeit ;
					double temperatur ;
					int doReset = 0 ;
					if (!CWusb.GetTemperature(devID, &temperatur, &zeit)) {
						if (debugEnabled)
							printf("GetTemperature(%d) failed\n", devID) ;
						doReset = 1 ;
						errorCount++ ;
						}
					else if (zeit == cwInfo[devID].tempTime) {
						if (debugEnabled)
							printf("USB-Temp(%d) freezed\n", devID) ;
						doReset = 1 ;
						errorCount++ ;
						}
					else
						errorCount = 0 ; ;
					if (doReset) {
						CWusb.ResetDevice(devID) ;
						cwInfo[devID].elapsedTime += 3000 ;		// wait a bit longer
						break ;
						}
					if (debugEnabled)
						printf("temperature at %d = %lf C\n", devID, temperatur) ;
					// send data
					static char sendStr[1024] ;
					sprintf(sendStr, "ClewareControl: %d, %d, %d, 0, 0, 0, %d;%d, %lf;\n", 
							cwInfo[devID].devType, cwInfo[devID].serialNumber, cwInfo[devID].version,
							remoteData, 0, temperatur) ;
							
					if (debugEnabled)
						printf("send <%s> to server\n", sendStr) ;
					rc = send(sd, sendStr, strlen(sendStr), 0) ;
					if (rc < 0 && debugEnabled) {
						printf("cannot send data to server (signal = %d)\n", cwSignal) ;
						// maybe we should reconnect
						}
					break ;
					}
				case CUSBaccess::HUMIDITY1_DEVICE: {
					int zeit ;
					double temperatur, humidity ;
					int doReset = 0 ;
					if (!CWusb.GetTemperature(devID, &temperatur, &zeit)) {
						if (debugEnabled)
							printf("GetTemperature(%d) failed\n", devID) ;
						doReset = 1 ;
						errorCount++ ;
						}
					else if (zeit == cwInfo[devID].tempTime) {
						if (debugEnabled)
							printf("USB-Temp(%d) freezed\n", devID) ;
						doReset = 1 ;
						errorCount++ ;
						}
					else
						errorCount = 0 ;
					if (!CWusb.GetHumidity(devID, &humidity, &zeit)) {
						if (debugEnabled)
							printf("GetHumidity(%d) failed\n", devID) ;
						doReset = 1 ;
						errorCount++ ;
						}
					else if (zeit == cwInfo[devID].tempTime) {
						if (debugEnabled)
							printf("USB-Temp(%d) freezed\n", devID) ;
						doReset = 1 ;
						errorCount++ ;
						}
					else
						errorCount = 0 ;
					if (doReset) {
						if (errorCount < 6) {
							if (debugEnabled)
								printf("try aagain later\n") ;
							cwInfo[devID].elapsedTime = 100 ;		// wait a short period of time
							break ;
						}
						if (errorCount > 12) {
							CWusb.ResetDevice(devID) ;
							errorCount = 6 ;			// don't reset too often
							break ;
							}
						CWusb.StartDevice(devID) ;
						cwInfo[devID].elapsedTime += 1000 ;		// wait a bit longer
						break ;
						}
					if (debugEnabled)
						printf("%d: temperature=%.2lf C, humidity=%.2lf RH\n", devID, temperatur, humidity) ;
					// send data
					static char sendStr[1024] ;
					sprintf(sendStr, "ClewareControl: %d, %d, %d, 0, 0, 0, %d;%d, %lf;\n", 
							cwInfo[devID].devType, cwInfo[devID].serialNumber, cwInfo[devID].version,
							remoteData, 0, humidity) ;
							
					if (debugEnabled)
						printf("send <%s> to server\n", sendStr) ;
					rc = send(sd, sendStr, strlen(sendStr), 0) ;
					if (rc < 0 && debugEnabled) {
						printf("cannot send data to server (signal = %d)\n", cwSignal) ;
						// maybe we should reconnect
						}
					sprintf(sendStr, "ClewareControl: %d, %d, %d, 0, 0, 0, %d;%d, %lf;\n", 
							CUSBaccess::TEMPERATURE5_DEVICE, cwInfo[devID].serialNumber, cwInfo[devID].version,
							remoteData, 0, temperatur) ;
							
					if (debugEnabled)
						printf("send <%s> to server\n", sendStr) ;
					rc = send(sd, sendStr, strlen(sendStr), 0) ;
					if (rc < 0 && debugEnabled) {
						printf("cannot send data to server (signal = %d)\n", cwSignal) ;
						// maybe we should reconnect
						}
					break ;
					}
				}
			}
		if (errorCount > USBcount + 33)
			cwSignal = 4711 ;
		Sleep(time2sleep) ;		// etwas warten
		}

	CWusb.CloseCleware() ;

	return 0;
	}

void
catchIOproblems(int sig) {
	cwSignal = sig ;
	}
