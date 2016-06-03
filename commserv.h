/*
 * commserv.h
 *
 *  Created on: Jun 1, 2016
 *      Author: lbc
 */

#ifndef COMMSERV_H_
#define COMMSERV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <wait.h>
void errorlog(char *,...);
void sig_cld();
int timeout;
int sem_id;
int rport;
char rhost[21];
void handtimeout();
char *handletime();
#define MSG_PERM 0777
#define FTOKSEQ 66
#define MAXQLEN 100
#define MAXPROC 20
#define PACKLEN 1024
#define SERVER_STARTUP 1
#define SERVER_SHUTDOWN 0
typedef struct{
	char mhost[21];
	int mport;
	char rhost[21];
	int rport;
}XMGL;


#endif /* COMMSERV_H_ */
