#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define RCVBUFSIZE 32
void DieWithError(char *errorMessage);
void HandleTCPClient(int cIntSocket) {

	char echoBuffer[RCVBUFSIZE];    /*Buffer for echo string*/
	int recvMsgSize=1;         /*Size of received message*/
	
	//Practice #1 make a log file
	FILE *fpForLog;
	char *fileName = "echo_history.log";
	char *refuseMessage = "Say hello!\n";
	//int connectAllowFlag = 0;

	if(recv(cIntSocket, echoBuffer, 32, 0) < 0)
		DieWithError("connect failed");
	if(strncmp(echoBuffer,"hello",5)==0){
		printf("%s\n",echoBuffer);
		if(send(cIntSocket, "hi", 2, 0) != 2)
			DieWithError("connect failed");}
	else{
		DieWithError("connect failed\n");
	}
	do{ 
		/* Receive message from client*/
		if ((recvMsgSize = recv(cIntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
			DieWithError("recv() failed");
		/* Send received string and receive again until end of transmission*/

		echoBuffer[recvMsgSize] = '\0';
		printf("msg<-%s\n",echoBuffer);    //print also to Serverside
		printf("msg->");

		/*print to file*/
		fpForLog = fopen(fileName,"a");
		fprintf(fpForLog, "%s\n", echoBuffer);
		fclose(fpForLog);
		
			/*Echo message back to client*/
		printf("%s\n",echoBuffer);
		if (send(cIntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
			DieWithError("send() failed");
				
		//}
	}while(recvMsgSize>0);/* zero indicates end of transmission*/
}
