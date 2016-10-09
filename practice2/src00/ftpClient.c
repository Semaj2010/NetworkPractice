#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define  EchoReq 	"EchoReq|" 
#define  FileUpReq	"FileUpReq|" 
#define  EchoRep 	"EchoRep|" 
#define  FileAck 	"FileAck|" 
#define  Delimeter	"|"
char msgType[10], recvChar;int i=0;
char * operation; // ex) client 203.252.164.144 upload test.txt 5000

int main(int argc, char *argv[]){

	int sock;	
	struct sockaddr_in ftpServAddr;
	unsigned short ftpServPort;
	char servIP[32];
	operation = argv[2];

	

	if(!strncmp(operation, "upload", 6)){
		srtcpy(msgType, FileUpReq);
		send(sock,msgType,strlen(msgType),0);
	}
	else if(!strncmp(operation,"echo",4)){
		strcpy(msgType, EchoReq);
		send(sock,msgType,strlen(msgType),0);
	}
	else{
		fprintf(stderr, "Usage: %s <Server IP> <operation><operand> <echo Port>\n",argv[0]);

		exit(1);
	}


}
