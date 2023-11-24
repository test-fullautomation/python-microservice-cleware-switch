/* definitions for send2cc
 *
 * (C) Copyright 2002 Cleware GmbH
 */

int cwLoop(char *serverName, int port, int debugEnabled) ;
void catchIOproblems(int sig) ;
typedef void (*sighandler_t)(int) ;

struct cwStruct {
	int tempTime ; 		  // time returned from USB-Temp
	int sleepTime ;      // time to wake up in ms
	int elapsedTime ;    // current time to wake up in ms
	int	devType ;
	int serialNumber ;
	int version ;
	} ;


enum {
	remoteData=10, remoteManualAction=11, remoteDisconnect=12, remoteInterval=13, remoteName=14
	} ;
