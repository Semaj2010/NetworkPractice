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
	int connectAllowFlag = 0;

	do{ 
		/* Receive message from client*/
		if ((recvMsgSize = recv(cIntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
			DieWithError("recv() failed");
		/* Send received string and receive again until end of transmission*/

		echoBuffer[recvMsgSize] = '\0';

		printf("%s",echoBuffer);    //print also to Serverside
		printf("\n");

		/*print to file*/
		fpForLog = fopen(fileName,"a");
		fprintf(fpForLog, "%s\n", echoBuffer);
		fclose(fpForLog);
		
		if(connectAllowFlag == 0){
			/*string compare for allow to connect. need hello*/	
			if(strncmp(echoBuffer, "hello",5)==0){
				send(cIntSocket,"Hi!\n",4,0);
				connectAllowFlag = 1;	
			}else{
				write(cIntSocket, refuseMessage, strlen(refuseMessage));
			}
		}
		else{
			/*Echo message back to client*/
			if (send(cIntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
				DieWithError("send() failed");
				
		}
	}while(recvMsgSize>0);/* zero indicates end of transmission*/
}
