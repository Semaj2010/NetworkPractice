#include <stdio.h>	/*for printf() and fprintf()*/
#include <sys/socket.h>	/*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h>	/*for sockaddr_in and inet_addr()*/
#include <stdlib.h>	/*for atoi() and exit()*/
#include <string.h>	/*for memset()*/
#include <unistd.h>	/*for close() */

#define RCVBUFSIZE 100	/*Size of receive buffer*/

void DieWithError(char *errorMessage);	/*Error handling function */
void do_socket(int socket);
void do_keyboard(int socket, pid_t p);

pid_t pid;

int main(int argc, char *argv[]){

	int sock;	/*Socket descriptor*/
	struct sockaddr_in echoServAddr; /*Echo server address*/
	unsigned short echoServPort; /*Echo server port*/
	char servIP[32];  /*Server IP address (dotted quad)*/
	char *echoString;  /*String to send to echo server*/
	char echoBuffer[RCVBUFSIZE];  /*Buffer for echo string*/
	unsigned int echoStringLen;  /*Length of string to echo*/
	int bytesRcvd, totalBytesRcvd; /*Bytes read in single recv() and total bytes read*/
	

	echoString = (char*)malloc(RCVBUFSIZE * sizeof(char));
	memset(echoString, 0x00, RCVBUFSIZE);

	/*Create a reliable, stream socket using TCP*/
	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) <0)
		DieWithError("socket() failed");

	printf("Server IP:");
	scanf("%s", servIP);
	if(servIP[0]=='\n')
		strcpy(servIP,"127.0.0.1");	
	printf("Server Port:");
	scanf("%hd", &echoServPort);

	/*Construct the server address structure*/
	memset(&echoServAddr, 0, sizeof(echoServAddr));  /*Zero out structure*/
	echoServAddr.sin_family    =AF_INET;    /*Internet address family*/
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /*Server IP address*/
	echoServAddr.sin_port    =htons(echoServPort);  /*Server port*/

	/*Establish the connection to the echo server*/
	if(connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError("connect failed");
	printf("Server IP : %s Port:%hd\n",inet_ntoa(echoServAddr.sin_addr),ntohs(echoServAddr.sin_port));

	if((pid = fork())< 0){
		printf("fork error\n");
		return -1;
	} else if(pid > 0){ 
		do_keyboard(sock, pid);
	} else if(pid == 0){
		do_socket(sock);
	}

	
	/*its for 3-way handshake*/
	if(send(sock, "hello", 5, 0) != 5)
		DieWithError("send() sent a different number of bytes than expected");
	if(recv(sock,echoString,RCVBUFSIZE,0)<0)
		DieWithError("recv() failed");
	printf("%s\n",echoString);
	/*communicate with a server*/

	for(;;){
		scanf("%s", echoString);

		if(strncmp(echoString,"/quit",5)==0){
			close(sock);
			printf("Close Connection..\n");
			break;
		}

		echoStringLen = strlen(echoString);    /*Determine input length*/
		printf("msg->%s\n",echoString);

		/*Send the string to the server*/
		if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
			DieWithError("send() sent a different number of bytes than expected");


		/*Receive the same string back from the server*/
		totalBytesRcvd = 0;
		printf("msg<-");        /*Setup to print the echoed string*/

		bytesRcvd = 1;
		do {
			/*Receive up to the buffer size (minus 1 to leave space for a null terminator) bytes from the sender*/
			if ((bytesRcvd =recv(sock, echoString, RCVBUFSIZE - 1, 0)) <= 0)
				DieWithError("recv() failed or connection closed prematurely");
			echoString[bytesRcvd] = '\0'; /*Terminate the rest string!*/
			printf("%s\n",echoString);
		}while(bytesRcvd >= (RCVBUFSIZE-1));
	}
	printf("Connection end\n"); /*Print a final linefeed*/
	exit(0);
	
}
