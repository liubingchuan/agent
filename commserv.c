/*
 * commserv.c
 *
 *  Created on: Jun 2, 2016
 *      Author: lbc
 */
#include "commserv.h"

main(argc, argv)
int argc;
char *argv[];
{
	key_t semkey;
	int nCmdFlag;
	int size, chid, len, i, pid, cpid, spid;
	int hsockfd, csockfd, n, c_len, mport;
	char lhost[21], ip[21], mhost[21];
	struct sockadd_in sin, c_add, s_add;
	struct msqid_ds mqst;
	struct sigaction act, oact;
	struct hostent *hp;
	XMGL *xmgl;
	xmgl = (XMGL *)malloc(sizeof(XMGL));
	bzero(xmgl, sizeof(XMGL));

	if((nCmdFlag = CheckCmdLine(argc, argv, xmgl))<0)
	{
		free(xmgl);
		return 1;
	}
	semkey = ftok("/etc/profile", FTOKSEQ);
	for(i=0;i<32;i++)
		if(i !=SIGKILL && i!=SIGBUS && i!=SIGALRM )
			signal(i, SIG_IGN);

	if(nCmdFlag == SERVER_STARTUP)
	{
		strcpy(mhost, xmgl->mhost);
		mport = xmgl->mport;
		gethostname(lhost, 20);
		hp = gethostbyname(lhost);
		memcpy((char *)&sin.sin_addr, (char *)hp->h_addr, hp->h_length);
		strcp(ip, inet_ntoa(sin.sin_addr));
		if(strcmp(mhost, lhost)&&strcmp(mhost, ip))
		{
			fprintf(stderr, "Service wasn't registered on local machine.[%s].\n",mhost);
			free(xmgl);
			exit(0);
		}
		strcpy(rhost, xmgl->rhost);
		rport = xmgl->rport;
		free(xmgl);
		pid = setpgrp();
		errorlog("Starting commserv...\n");
		if((sem_id = sem_create(semkey, 1))<0)
		{
			errorlog("Sem_create failed.\nStart failed.\n");
			exit(-1);
		}
		spid = get_pid(semkey, 1, 0);
		if(spid>0&&spid!=getpid()){
			fprintf(stderr, "The system has been started.\n");
			exit(-1);
		}
		set_sem(sem_id, 0, MAXPROC);
		errorlog("Startup Done.\n");
	}
	else
	{
		 free(xmgl);
		 if((sem_id=sem_create(semkey, 1))<0)
		 {
			 errorlog("Sem_create failed.\nStart failed.\n");
			 exit(-1);
		 }
		 errorlog("Stopping commserv...\n");
		 pid=get_pid(semkey, 1, 0);
		 pid=(-1)*pid;
		 free_sem(sem_id);
		 errorlog("Shutdown Down.\n");
		 kill(pid, SIGKILL);
		 exit(0);
	}

	if((cpid=fork())<0)
	{
		errorlog("Fork:%s\n",strerror(errno));
		exit(-1);
	}
	if(cpid != 0)
		exit(0);
	signal(SIGALRM, handletimeout);
	for(;;)
	{
		if((hsockfd = socket(PF_INET, SOCK_STREAM, 0))<0)
		{
			errorlog("Socket:%s\n",strerror(errno));
			sleep(2);
		}
		else break;
	}
	bzero((char *)&s_add, sizeof(s_add));
	s_add.sin_family = AF_INET;
	s_add.sin_addr.s_addr = htonl(INADDR_ANY);
	s_add.sin_port = htons(mport);
	size = 1;
	len = sizeof(size);
	setsockopt(hsockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&size, len);
	for(;;)
	{
		if(bind(hsockfd, (struct sockaddr *)&s_add, sizeof(s_add))<0)
		{
			errorlog("Bind:%s\n",strerror(errno));
			sleep(2);
		}
		else break;
	}

	listen(hsockfd, 5);

	act.sa_handler = sig_cld;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGCLD, &act, &oact);
	for(;;)
	{
		c_len = sizeof(c_add);
		csockfd = accept(hsockfd, (struct sockaddr *)&c_add, &c_len);
		if(csockfd<0)
		{
			if(errno==EINTR) continue;
			errorlog("Accept:%s\n",strerror(errno));
			close(csockfd);
			continue;
		}
		if(sem_wait(sem_id, 0))
		{
			errorlog("Can't fork too many processes.");
			close(csockfd);
			continue;
		}
		chid = 0;
		if((chid=fork())!=0)
		{
			if(chid==-1)
			{
				errorlog("Fork:%s\n",strerror(errno));
				sem_signal(sem_id, 0);
			}
			close(csockfd);
			continue;
		}
		close(hsockfd);
		handle_request(csockfd);
		close(csockfd);
		exit(0);
	}
}

void handtimeout()
{
	timeout = 1;
}

void sig_cld(int signo)
{
	int status;
	while(waitpid(-1, &status, WNOHANG)>0){
		sem_signal(sem_id, 0);
	}
}

int handle_request(int csockfd)
{
	int n, buflen, sockfd;
	char buf[PACKLEN+1],sendbuf[PACKLEN+1],retbuf[PACKLEN+1],lenbuf[5];
	struct hostent *hp;
	struct sockaddr_in sin;
	memset((void *)buf, 0, sizeof(buf));
	n = readsocket(csockfd, buf, 4);
	if( n != 4)
	{
		errorlog("Read error");
		return (-1);
	}
	buf[4]=0;
	buflen=atoi(buf);
	if(readsocket(csockfd,buf,buflen)!=buflen)
	{
		errorlog("Read error");
		return (-2);
	}
	errorlog("Received site's text:%s\n",buf);
	memset(sendbuf,0x00,sizeof(sendbuf));
	sprintf(sendbuf,"%04d%s",buflen, buf);
	errorlog("Send the request text to the delegate corporation:%s\n",sendbuf);
	if((sockfd = socket(PF_INET,SOCK_STREAM,0))<0)
	{
		errorlog("Socket:%s\n",strerror(errno));
		return (-4);
	}
	if((hp=gethostbyname(rhost))==NULL)
	{
		errorlog("Gethostbyname:%s\n",strerror(errno));
		close(sockfd);
		return (-5);
	}
}

