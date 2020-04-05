
/*
 *	msgutil.c -- System V IPC message queue utility
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "msgq.h"

int open_queue( key_t keyval, int flag )
{
	int     qid;
	
	if (flag == MQ_CREATE) {
		/* Create the queue - fail if exists */
		if((qid = msgget( keyval, IPC_CREAT |IPC_EXCL | 0660 )) == -1)
		{
			return(-1);
		}
	}
	else {
		/* Get the queue id - fail if not exists */
		if((qid = msgget( keyval, 0660 )) == -1)
		{
			return(-1);
		}
	}
	
	return(qid);
}

void send_message(int qid, long type, long req, MSG_T *msg)
{
	struct mymsgbuf qbuf;
	
	/* Send a message to the queue */
	//printf("Sending a message ... qid=%d, mtype=%d\n", qid, type);
	qbuf.mtype = type;
	qbuf.request = req;
	qbuf.msg.cmd = msg->cmd;
	qbuf.msg.arg1 = msg->arg1;
	qbuf.msg.arg2 = msg->arg2;
	memcpy(qbuf.msg.mtext, msg->mtext, MAX_SEND_SIZE);
	
	if((msgsnd(qid, &qbuf, sizeof(struct mymsgbuf)-sizeof(long), 0)) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	return;
}

void read_message(int qid, struct mymsgbuf *qbuf, long type)
{
	int ret;
	
	/* Read a message from the queue */
	//printf("Reading a message ...\n");
	qbuf->mtype = type;
	ret=msgrcv(qid, (struct msgbuf *)qbuf,
		sizeof(struct mymsgbuf)-sizeof(long), type, 0);
/*
	if (ret == -1) {	
		switch (errno) {
			case E2BIG   :
				printf("E2BIG    \n");
				break;
			case EACCES :
				printf("EACCES  \n");
				break;
			case EFAULT   :
				printf("EFAULT   \n");
				break;
			case EIDRM  :
				printf("EIDRM  \n");
				break;
			case EINTR    :
				printf("EINTR    \n");
				break;
			case EINVAL   :
				printf("EINVAL   \n");
				break;
			case ENOMSG   :
				printf("ENOMSG   \n");
				break;
			default:
				printf("unknown\n");
		}
	}
	printf("Type: %ld Text: %s\n", qbuf->mtype, qbuf->mtext);
*/
}

//#ifdef MSG_WLAN_RESTART_SUPPORT
void send_message_boa(int qid, long type, long req, BOA_MSG_T *msg)
{
	struct boamsgbuf qbuf;
	
	/* Send a message to the queue */
	//printf("Sending a message ... qid=%d, mtype=%d\n", qid, type);
	qbuf.mtype = type;
	qbuf.request = req;
	qbuf.msg.cmd = msg->cmd;
	qbuf.msg.arg1 = msg->arg1;
	qbuf.msg.arg2 = msg->arg2;
	memcpy(qbuf.msg.mtext, msg->mtext, BOA_MAX_SEND_SIZE);
	
	if((msgsnd(qid, &qbuf, sizeof(struct boamsgbuf)-sizeof(long), 0)) == -1)
	{
		perror("msgsnd");
		exit(1);
	}
	return;
}

//void read_message_nowait(int qid, struct mymsgbuf *qbuf, long type)
void read_message_nowait(int qid, struct boamsgbuf *qbuf, long type)
{
	int ret;
	
	/* Read a message from the queue with NOWAIT flag*/
	qbuf->mtype = type;
//	ret=msgrcv(qid, (struct msgbuf *)qbuf,
//		sizeof(struct mymsgbuf)-sizeof(long), type, IPC_NOWAIT);
	ret=msgrcv(qid, (struct msgbuf *)qbuf,
		sizeof(struct boamsgbuf)-sizeof(long), type, IPC_NOWAIT);
	return ret;
}
//#endif

int peek_message( int qid, long type )
{
        int     result, length;

        if((result = msgrcv( qid, NULL, 0, type,  IPC_NOWAIT)) == -1)
        {
                if(errno == E2BIG)
                        return(1);
        }
        
        return(0);
}

void remove_queue(int qid)
{
	/* Remove the queue */
	msgctl(qid, IPC_RMID, 0);
}

