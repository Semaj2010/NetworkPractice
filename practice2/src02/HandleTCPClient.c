#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#define RCVBUFSIZE 100

void DieWithError(char *errorMessage);
void do_keyboard(int c_socket, pid_t pid);
void do_socket(int c_socket);

void do_keyboard(int c_socket, pid_t pid){
	int n;
	char sbuf[RCVBUFSIZE];

	while((n = read(0, sbuf, RCVBUFSIZE)) > 0){
		printf("keyboard<-%s",sbuf);
		if(write(c_socket, sbuf, n) != n) {
			printf("Talk Server fail in sending\n");
		}
		if(strncmp(sbuf, "quit", 4)) {
			kill(pid, SIGQUIT);
			break;
		}	
	}
}
void do_socket(int c_socket){
	int recv_n;
	char rbuf[RCVBUFSIZE];
	FILE *fpForLog;
	char *logFileName = "echo_history.log";

	if((recv_n = read(c_socket, rbuf, RCVBUFSIZE)) < 0 )
		DieWithError("handsahke failed");
	if(strncmp(rbuf,"hello",5)==0){
		printf("%s\n",rbuf);
		if(send(c_socket, "hi", 2, 0) != 2)
			DieWithError("handshake failed");}
	else{
		DieWithError("handshake failed");
	}

	while(1){
			if((recv_n = read(c_socket, rbuf, RCVBUFSIZE)) > 0){
			rbuf[recv_n] = '\0';
			printf("msg<-%s", rbuf);

			fpForLog = fopen(logFileName,"a");
			fprintf(fpForLog, "%s\n", rbuf);
			fclose(fpForLog);

			if(strncmp(rbuf, "quit", 4) == 0){
				kill(getppid(), SIGQUIT);
				break;
			}
			if(write(c_socket, rbuf, recv_n) < 0 )
				DieWithError("send error");
			printf("msg->%s",rbuf);
		}
	}
}
void HandleTCPClient(int cIntSocket) {
	char echoBuffer[RCVBUFSIZE];    /*Buffer for echo string*/
	int recvMsgSize=1;         /*Size of received message*/
		
	//Practice #1 make a log file
	FILE *fpForLog;
	char *logFileName = "echo_history.log";
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
		fpForLog = fopen(logFileName,"a");
		fprintf(fpForLog, "%s\n", echoBuffer);
		fclose(fpForLog);
		
			/*Echo message back to client*/
		printf("%s\n",echoBuffer);
		if (send(cIntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
			DieWithError("send() failed");	
		//}
	}while(recvMsgSize>0);/* zero indicates end of transmission*/
}
