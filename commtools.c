/*
 * commtools.c
 *
 *  Created on: Jun 1, 2016
 *      Author: lbc
 */
#include "commserv.h"
#include <getopt.h>

char *lrtrim(char *buf)
{
	int len;
	char *p;
	while(isspace(*buf++));
	p=buf;
	len=strlen(p);
	while(isspace(p[--len])) p[len]=0;
	return (p);
}

void errorlog(char *fmt,...)
{
	FILE *fp;
	char *rq,yr[6],fname[11];
	va_list para;
	va_start(para,fmt);
	rq=(char *)handletime(0);
	strncpy(yr,rq,5);
	yr[5]=0;
	sprintf(fname,"./log%s",yr);
	fp=fopen(fname,"a");
	fprintf(fp,"[commserv][%s]:",rq);
	vfprintf(fp,fmt,para);
	fclose(fp);
	va_end(para);
}

char *handletime(int bz)
{
	struct tm *tp;
	long secs;
	char *buff;
	buff=(char *)malloc(30);
	secs=time((long *)0);
	tp=localtime(&secs);
	switch(bz){
		case 0:
			sprintf(buff,"%02d.%02d %02d:%02d:%02d",tp->tm_mon+1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
			break;
		case 1:
			sprintf(buff,"%04d%02d%02d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday);
			break;
		case 2:
			sprintf(buff,"%04d%02d%02d%02d%02d%02d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_yday, tp->tm_hour, tp->tm_min, tp->tm_sec);
			break;
		default:
			sprintf(buff,"%04d%02d%02d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday);
			break;
	}
	return (buff);
}


int readpara(char *filename, XMGL *xs)
{
	int i;
	FILE *fp;
	char buf[201];

	if((fp=fopen(filename,"r"))==NULL)
	{
		errorlog("Open commserv.cfg error.");
		return (-1);

	}

	if(fgets(buf,100,fp)==NULL) return (-1);
	sscanf(buf,"%s %d %s %d", xs->mhost, &xs->mport, xs->rhost, &xs->rport);
	return (0);
}

int CheckCmdLine(int gc,char *gv[],XMGL *xs)
{
	char c;
	int nFlag=-1;
	char *opt_arg_value=NULL;
	struct option longopts[]={
			{"startup",no_argument,NULL,'s'},
			{"shutdown",no_argument,NULL,'x'},
			{"config",required_argument,NULL,'c'},
			{NULL,0,NULL,0},
	};
	while((c=getopt_long(gc,gv,":sxc:",longopts,NULL))!=-1){
		switch(c)
		{
			case 's':
				nFlag = SERVER_STARTUP;
				break;
			case 'x':
				nFlag = SERVER_SHUTDOWN;
				break;
			case 'c':
				opt_arg_value = optarg;
				break;
			case ':':
				fprintf(stderr,"argument %c need value.\n",optopt);
				break;
			case '?':
				fprintf(stderr,"Invalidate argument %c.\n",optopt);
				break;
			default:
				break;
		}
	}
	if(nFlag==-1 || opt_arg_value==NULL)
	{
		fprintf(stderr,"Usage:%s [-startup{-s}|-shutdown{-x}] -config{-c} filename \n",gv[0]);
		return (-1);
	}
	if(opt_arg_value != NULL)
	{
		readpara(opt_arg_value,xs);
	}
	return nFlag;
}

msg_snd(int mid,void *msg_text,int len)
{
	int i;
	for(i=0;i<5;i++)
	{
		if(!msgsnd(mid,(void *)msg_text,len, IPC_NOWAIT))
			return;
	}
}

int sem_create(key_t key, int num_sem)
{
	int id,i;
	if(key == IPC_PRIVATE)
		return (-1);
	else if(key == (key_t)-1)
		return -1;

	if((id=semget(key,num_sem,0666|IPC_CREAT|IPC_EXCL))<0)
	{
		if(errno == EEXIST)
		{
			if((id=semget(key,num_sem,0))<0)
				return (-1);
			else return(id);
		}
		else return (-1);
	}
	return id;
}

int set_sem(int id,int sem_no,int init_val)
{
	union semun {
		int val;
		struct semid_ds *buf;
		ushort *array;
	}semctl_arg;
	semctl_arg.val = init_val;
	if(semctl(id,sem_no,SETVAL,semctl_arg)<0)
	{
		return (-1);
	}
	else
		return (0);
}

int get_sem(int id, int sem_no)
{
	union semun {
		int val;
		struct semid_ds *buf;
		ushort *array;
	}semctl_arg;

	return(semctl(id,sem_no,GETVAL,semctl_arg));
}

int sem_op(int id,int num_sem,int amount)
{
	int fh=0;
	struct sembuf op_op[1];
	op_op[0].sem_num = num_sem;
	op_op[0].sem_op = amount;
	op_op[0].sem_flg |= IPC_NOWAIT;
	fh = semop(id,&op_op[0],1);
	return fh;
}

int sem_wait(int id, int num_sem)
{
	int fh=0;
	fh = sem_op(id, num_sem, -1);
	return fh;
}

int sem_signal(int id, int num_sem)
{
	int fh = 0;
	fh = sem_op(id, num_sem, 1);
	return fh;
}

void free_sem(int semid)
{
	sem_signal(semid, 0);
	semctl(semid, 0, IPC_RMID);
}

int get_pid(long key, int semnum, int sem_no)
{
	int semid;
	int id;
	union semun {
		int val;
		struct semid_ds *buf;
		ushort *array;
	}arg;
	if((semid=semget((key_t)key, semnum, 0))<0)
		 return (-1);
	if((id=semctl(semid, sem_no, GETPID, arg))<0)
		 return (-1);
	else
		return (id);
}
