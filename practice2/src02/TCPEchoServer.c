/*Header same with client!*/
#include <stdio.h>	/*for printf() and fprintf()*/
#include <sys/socket.h>	/*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h>	/*for sockaddr_in and inet_addr()*/
#include <stdlib.h>	/*for atoi() and exit()*/
#include <string.h>	/*for memset()*/
#include <unistd.h>	/*for close() */
#include <signal.h>

#define MAXPENDING 5    /*Maximum outstanding connection requests*/

void DieWithError(char *errormessage); /*Error handling function*/
int  HandleTCPClient(int cIntSocket);  /*TCP client handling function*/
void do_keyboard(int socket,pid_t p);
void do_socket(int socket);

pid_t pid;	//process id

int main(int argc, char *argv[]){

	int servSock;           /*Socket descriptor for server*/
	int cIntSock;           /*Socket descriptor for client*/
	struct sockaddr_in echoServAddr; /*Local address*/
	struct sockaddr_in echoCIntAddr; /*Client address*/
	unsigned short echoServPort;     /*Server Port*/
	unsigned int cIntLen;           /*Length of client address data structure*/

	if (argc != 2) {   /*Test for correct number of arguments*/

		fprintf(stderr, "Usage :  %s <Server Port>\n", argv[0]);
		exit(1);
	}
	/*Create socket for incoming connections*/
	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");

	echoServPort = atoi(argv[1]); /*First arg: local port*/

	/*Construct local address structure*/
	memset(&echoServAddr, 0, sizeof(echoServAddr));    /*Zero out structure*/
	echoServAddr.sin_family = AF_INET;         /*Internet address family*/
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /*Any incoming interface*/

	echoServAddr.sin_port = htons(echoServPort);    /*Local port*/

	/*Bind to the local address */
	if (bind(servSock, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) < 0 )
		DieWithError("bind() failed");

	/*Mark the socket so it will listen for incoming connections*/
	if (listen(servSock, MAXPENDING) < 0)
		DieWithError("listen() failed");
	
	/*Set the size of the in-out parameter*/
	cIntLen = sizeof(echoCIntAddr);

	for(;;){ /*wait for new connection from client*/
		printf("Wait for new connect at port:%hd...\n",ntohs(echoServAddr.sin_port));
		/*Wait for a client to connect*/
		if((cIntSock = accept(servSock, (struct sockaddr *) &echoCIntAddr, &cIntLen)) < 0)
			DieWithError("accept() failed");
		/*cIntSock is connected to a client!*/
		printf("Client ip address : %s\n", inet_ntoa(echoCIntAddr.sin_addr));
		printf("Client port : %d\n", (int)(ntohs(echoCIntAddr.sin_port)));

		if((pid = fork() < 0)){
			printf("fork error\n");
		} else if(pid > 0){
			do_keyboard(cIntSock,pid);
		} else if(pid == 0){
			do_socket(cIntSock);
		}
		//printf("Handling client %s\n", inet_ntoa(echoCIntAddr.sin_addr));
	
		//HandleTCPClient(cIntSock);
	}

}

