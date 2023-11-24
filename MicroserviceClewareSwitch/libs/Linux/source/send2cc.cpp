// send2cc.cpp :
// send infos of all found cleware devices to server
//		the arguments:
//		-s servername	the name or ip address is supplied as an argument
//		-p IP-port		Port number, default = 54741 
//		-r				retry in case of error
//		-d				print debug info
//		-v				print version number
//		-h				print help info
//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "send2cc.h"

const int defaultPort = 54741 ;
const char *progVersion = "1.2" ;

int 
main(int argc, char* argv[]) {
	char *serverName = 0 ;
	int port = defaultPort ;
	int retry = 0 ;
	int debugEnabled = 0 ;
	int printHelp = 0 ;
	int printVersion = 0 ;
	char *progName = *argv ;

	for (argc--, argv++ ; argc > 0 ; argc--, argv++) {
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
				case 's':
				case 'S':
					if (argc > 1) {
						serverName = argv[1] ;
						argc-- ;
						argv++ ;
						}
					break ;
				case 'd':
				case 'D':
					debugEnabled = 1 ;
					break ;
				case 'p':
				case 'P': {
					int newPort = 0 ;
					if (argc <= 1) {
						printf("missing port number\n") ;
						printHelp = 1 ;
						break ;
						}
					if (sscanf(argv[1], "%d", &newPort) != 1) {
						printf("illegal port number %s\n", argv[1]) ;
						printHelp = 1 ;
						}
					else
						port = newPort ;
					argc-- ;
					argv++ ;
					break ;
					}
				case 'v':
				case 'V':
					printVersion = 1 ;
					break ;
				case 'h':
				case 'H':
					printHelp = 1 ;
					break ;
				case 'r':
				case 'R':
					retry = 10 ;
					break ;
				default:
					printf("illegal argument %c\n", argv[0][1]) ;
					printHelp = 1 ;
					break ;
				}
			}
		}

	if (printVersion || debugEnabled)
		printf("%s version %s\n", progName, progVersion) ;

	if (serverName == 0 && printHelp == 0) {
		if (printVersion)
			return 0 ;
		printf("missing server name\n") ;
		printHelp = 1 ;
		}

	if (printHelp) {
		printf("Usage: %s -s servername {-d}\n", progName) ;
		printf("       -s   specify the ClewareControl server\n") ;
		printf("       -p   specify the port (default %d)\n", defaultPort) ;
		printf("       -r   retry in case of error\n") ;
		printf("       -d   print debug infos\n") ;
		printf("       -v   print version number\n") ;
		printf("       -h   print this\n") ;
		return 0 ;
		}

	while (1) {
		cwLoop(serverName, port, debugEnabled) ;
		if (!retry) 
			break ;
		if (debugEnabled)
			printf("something gone wrong - retry in %d seconds\n", retry) ;
		sleep(retry) ;		// retry every some seconds
		}

	exit(1) ;
	}

