#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#define  EchoReq 	"EchoReq|" 
#define  FileUpReq	"FileUpReq|" 
#define  EchoRep 	"EchoRep|" 
#define  FileAck 	"FileAck|" 
#define  Delimeter	"|"
char msgType[10], recvChar;int i=0;
char * operation; // ex) client 203.252.164.144 upload test.txt 5000

void DieWithError(char *message);
int main(int argc, char *argv[]){
	int c_socket, s_socket;	//client, server socket
	struct sockaddr_in s_addr, c_addr;
	unsigned short s_port;
	char fileName[100];
	unsigned int cIntLen;
	int i;
	int n;

	s_port = atoi(argv[1]);

	/*new socket. domain, type, protocol*/
	if((s_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");

	memset(&s_addr, 0, sizeof(s_addr));

	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(s_port);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);


	/*Bind to the local address */
	if(bind(s_socket,(struct sockaddr*)&s_addr, sizeof(s_addr))< 0)
		DieWithError("bind() failed");
	/*Mark the socket so it will listen for incoming connections*/
	if(listen(s_socket, 32) < 0)
		DieWithError("listen() failed");
	cIntLen = sizeof(c_addr);
	while(1){
		/*read the message, and if client request ftp, change to ftp
		 * and read the file which sent by client*/
		while(i<10){
			recvChar = recv(c_socket, msgType[i], sizeof(char),0);
			if(msgType[i] == Delimeter)break;

		}
		if(!(strcmp(msgType,FileUpReq))){
			recv(c_socket, fileName, ,0);
		}
	}

	


		exit(1);
	

	return 0;
}
