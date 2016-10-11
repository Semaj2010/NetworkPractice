#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>	/*for sockaddr_in and inet_addr()*/
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#define  EchoReq 	"EchoReq|" 
#define  FileUpReq	"FileUpReq|" 
#define  EchoRep 	"EchoRep|" 
#define  FileAck 	"FileAck|" 
#define  Delimeter	'|'
#define  Quit       "quit|"
#define RCVBUFSIZE 100
#define BUFSIZE    1000

char * operation; // ex) client 203.252.164.144 upload test.txt 5000
char msgType[15];
char recvChar;
void DieWithError(char *message);
void * do_echo(void *data);
void record_log(char *);
int ft_mode(int c_socket);

int main(int argc, char *argv[]){
	int c_socket, s_socket;	//client, server socket
	struct sockaddr_in s_addr, c_addr;
	unsigned short s_port;
	char fileName[100];
	unsigned int clntLen;
	char rcvBuffer[RCVBUFSIZE];
	char fileBuffer[BUFSIZE];
	int origFileSize, recvFileSize;
	FILE *fp;
	int recv_n;
	int i;

	/*thread 변수*/

	if(argc != 2)
		printf("Usage : ftpServer [Port]\n");

	s_port = atoi(argv[1]);

	/*new socket. domain, type, protocol*/
	if((s_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");

	memset(&s_addr, 0, sizeof(s_addr));
	/*init addr variable*/
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(s_port);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);


	/*Bind to the local address */
	if(bind(s_socket,(struct sockaddr*)&s_addr, sizeof(s_addr))< 0)
		DieWithError("bind() failed");
	/*Mark the socket so it will listen for incoming connections*/
	if(listen(s_socket, 32) < 0)
		DieWithError("listen() failed");
	//clntLen = sizeof(c_addr);
	while(1){
		printf("Wait for new connect... socket : %d \n", s_socket);
		clntLen = sizeof(c_addr);	
		if((c_socket = accept(s_socket, (struct sockaddr*)&c_addr, &clntLen)) < 0)	
			DieWithError("accept error");

		printf("client ip : %s\n", inet_ntoa(c_addr.sin_addr));
		printf("clinet port: %d\n", ntohs(c_addr.sin_port));

		if(recv(c_socket, rcvBuffer, 32, 0) < 0)
			DieWithError("connect failed");
		if(strncmp(rcvBuffer,"hello",5)==0){
			printf("%s\n",rcvBuffer);
			if(send(c_socket, "hi", 2, 0) != 2)
				DieWithError("connect failed");
		}
		else{
			DieWithError("connect failed");
		}
		//connect success!
		while(1){
			i=0;
			while(i < 10){
				if((recvChar = read(c_socket, msgType+i, sizeof(char))) < 0 )
					DieWithError("receive error");
				if(msgType[i] == Delimeter) break;
				i++;
			}
			msgType[i+1] = '\0';
			record_log(msgType);
			if(!(strncmp(msgType,FileUpReq,9))){
				/*File upload mode*/
				i=0;
				printf("File Transfer mode\n");
				while(i<RCVBUFSIZE){
					if((recvChar = read(c_socket,fileName+i,sizeof(char))) == 0)
						DieWithError("receive error");
					if(fileName[i++] == Delimeter) break;
				}
				fileName[i-1] = '\0';
				recv_n = read(c_socket,&origFileSize,sizeof(int));
				record_log(fileName);
				if(recv_n != 4)
					DieWithError("its not filesize");
				printf("FT<-File Name: %s / File Size : %d\n", fileName, origFileSize);
				/*File ACK send*/
				printf("FT->%s\n",FileAck);
				write(c_socket, FileAck, strlen(FileAck));

				fp = fopen("test/test.txt","w");
				/*receive file contents*/
				printf("File Receiveing <====");
				recvFileSize = 0;
				memset(fileBuffer,0x00,BUFSIZE);
				while((recv_n = read(c_socket, fileBuffer, BUFSIZE)) != 0){
					//printf("recv_n : %d\n", recv_n);
					fwrite(fileBuffer,sizeof(char),origFileSize,fp);	
					printf("####");
					if(origFileSize <= recv_n)
						break;
				}
				/*while(origFileSize > recvFileSize){
					if((recv_n = recv(c_socket, fileBuffer, BUFSIZE,0) < 0 ))
						DieWithError("recv failed in ft_mode");
					fwrite(fileBuffer, sizeof(char), BUFSIZE, fp);
					recvFileSize = recvFileSize + origFileSize;
					printf("origFilesize : %d / recvFileSize : %d\n",origFileSize,recvFileSize);
					if(recv_n == 0)
						break;
					sleep(10);
				}*/
				fclose(fp);
				printf("%s(%d bytes) uploading success from Client\n",fileName,origFileSize);
			}else if(!strncmp(msgType,EchoReq,8)){
				recv_n = read(c_socket,rcvBuffer,RCVBUFSIZE);
				rcvBuffer[recv_n] = '\0';
				record_log(rcvBuffer);
				printf("msg<-%s\n",rcvBuffer);
				//strcpy(msgType, EchoRep);
				write(c_socket, msgType, strlen(msgType));
				write(c_socket, rcvBuffer, strlen(rcvBuffer));
				printf("msg->%s\n",rcvBuffer);
			}else if(!strcmp(msgType,Quit)){
				printf("client quit\n");
				break;
			}
		}
	}

	printf("close server socket\n");
	close(s_socket);
	exit(1);


	return 0;
}
void record_log(char * data){
	char *log_file_name = "echo_history.log";
	FILE *fp;
	fp = fopen(log_file_name, "a");
	fprintf(fp, "%s\n", data);
	fclose(fp);
}

