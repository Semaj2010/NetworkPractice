#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#define RCVBUFSIZE 32
void DieWithError(char *errorMessage);
void HandleTCPClient(int cIntSocket) {

	char echoBuffer[RCVBUFSIZE];    /*Buffer for echo string*/
	int recvMsgSize;         /*Size of received message*/
	/* Receive message from client*/
	if ((recvMsgSize = recv(cIntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");
	/* Send received string and receive again until end of transmission*/
	while (recvMsgSize > 0){ /* zero indicates end of transmission*/
		/*Echo message back to client*/
		if (send(cIntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
			DieWithError("send() failed");
		/* See if there is more data to receive*/
		if ((recvMsgSize = recv(cIntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
			DieWithError("recv() failed");
	}		
	close(cIntSocket);    /*close client socket*/

}
