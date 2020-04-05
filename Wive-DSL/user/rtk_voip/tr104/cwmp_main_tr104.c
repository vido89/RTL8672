#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "cwmpevt.h"

#define CWMP_CLINET_PATH "/var/cwmpclient.chanl"
#define SOLAR_CHANNEL_PATH "/var/solar.chanl"
static int ipcSocket = 0;
cwmpEvtMsg pEvtMsg;

/*init the solar listener*/
int cwmp_solarInit(void){
	struct sockaddr_un ipcAddr;

	printf("+++++go to cwmp_solarInit+++++\n");
	ipcSocket = socket(PF_LOCAL, SOCK_DGRAM, 0);
	if(0 > ipcSocket){
		perror("Error:open IPC socket fail");
		return 0;
	}
	unlink(CWMP_CLINET_PATH);
	bzero(&ipcAddr, sizeof(ipcAddr));
	ipcAddr.sun_family = PF_LOCAL;
	strncpy(ipcAddr.sun_path, CWMP_CLINET_PATH, sizeof(CWMP_CLINET_PATH));
	if(bind(ipcSocket, (struct sockaddr*)&ipcAddr, sizeof(ipcAddr)) == -1){
		close(ipcSocket);
		perror("Error:bind IPC socket fail");
		return 0;
	}

	return 1;
}

/*create the solar listener thread*/
void *cwmp_solarListener(void *data){
	if(cwmp_solarInit()){
		printf("+++++go to cwmp_solarListener+++++\n");
		while(1){
			unsigned char *buf = NULL;
			if((buf = (unsigned char*)malloc(cwmpEvtMsgSizeof())) != NULL){
				cwmpEvtMsg *evtMsg;
				if (recvfrom(ipcSocket, (void*)buf, cwmpEvtMsgSizeof(), 0, NULL, NULL) > 1){
					evtMsg = (cwmpEvtMsg*) buf;
					if(EVT_VOICEPROFILE_LINE_SET_STATUS == evtMsg->event){
						memcpy((void*)&pEvtMsg, (void*)evtMsg,  cwmpEvtMsgSizeof());
					}
					printf("+++++recv string+++++\n");
				}
				free(buf);
			}
		}
	}
	//printf("+++++go to cwmp_solarListener+++++\n");
	
	return NULL;
}


/*open the connection from solar to cwmpclient*/
void cwmp_solarOpen( void )
{
	pthread_t cwmp_solar_pid;
	
	if( pthread_create(&cwmp_solar_pid, NULL, cwmp_solarListener, NULL) != 0)
		fprintf(stderr,"Error:initial solar listener fail\n");
}

/*close the connection from solar to cwmpclient*/
void cwmp_solarClose(void){
	close(ipcSocket);
}

/* send the request to solar */
void cwmpSendRequestToSolar(void){
	cwmpEvtMsg *sendEvtMsg=NULL;
	int sendSock=0;
	struct sockaddr_un addr;

	printf("+++++cwmpSendRequestToSolar\n+++++\n");
	if((sendEvtMsg = cwmpEvtMsgNew()) != NULL){
		sendEvtMsg->event = EVT_VOICEPROFILE_LINE_GET_STATUS;
		sendSock=socket(PF_LOCAL, SOCK_DGRAM, 0);
		bzero(&addr, sizeof(addr));
		addr.sun_family = PF_LOCAL;
		strncpy(addr.sun_path, SOLAR_CHANNEL_PATH, sizeof(SOLAR_CHANNEL_PATH));
		sendto(sendSock, (void*)sendEvtMsg, cwmpEvtMsgSizeof(), 0, (struct sockaddr*)&addr, sizeof(addr));
		close(sendSock);
		free(sendEvtMsg);
	}
}

