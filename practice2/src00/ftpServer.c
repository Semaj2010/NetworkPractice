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
#define  Quit       "quit"
#define RCVBUFSIZE 100
#define BUFSIZE    1000

char msgType[10], recvChar;int i=0;
char * operation; // ex) client 203.252.164.144 upload test.txt 5000

void DieWithError(char *message);
void * do_echo(void *data);
void record_log(FILE * fp, char *);
int ft_mode(int c_socket);

int clnt_cnt= 0;
int c_sockets[10];

pthread_mutex_t mtx; 
pthread_t pid[10];

int main(int argc, char *argv[]){
	int c_socket, s_socket;	//client, server socket
	struct sockaddr_in s_addr, c_addr;
	unsigned short s_port;
	char fileName[100];
	unsigned int clntLen;

	/*thread 변수*/

	pthread_t pthread1;
	int thr_id, status;

	if(argc < 1)
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

		pthread_mutex_lock(&mtx);

		c_sockets[clnt_cnt] = c_socket;

		pthread_mutex_unlock(&mtx);

		/*thread create! do_echo function take the process of read the client message and write to client back*/
		thr_id = pthread_create(&pid[clnt_cnt], NULL, &do_echo, (void*)c_socket);

		///pthread_join(pthread1, (void**) &status); 
		/*read the message, and if client request ftp, change to ftp
		while(i<10){
			recvChar = recv(c_socket, msgType[i], sizeof(char),0);
			if(msgType[i] == Delimeter)break;

		}
		if(!(strcmp(msgType,FileUpReq))){
			recv(c_socket, fileName, ,0);
		}
		*/
		pthread_join(pid[clnt_cnt], (void**) &status);
		clnt_cnt++;
	}
	close(s_socket);
	exit(1);
	

	return 0;
}

void * do_echo(void *data){
	int recv_n;
	char rcvBuffer[RCVBUFSIZE];
	FILE *fp;
	int c_socket =(int)data;
	int i=0;
	char fileName[RCVBUFSIZE];
	int origFileSize;
	int recvFileSize;
	char fileBuffer[BUFSIZE];

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
			if((recvChar = read(c_socket, msgType+i, sizeof(char))) == 0 )
				DieWithError("receive error");
			if(msgType[i++] == Delimeter) break;
		}
		if(!(strcmp(msgType,FileUpReq))){
			/*File upload mode*/
			i=0;
			while(i<RCVBUFSIZE){
				if((recvChar = read(c_socket,fileName+i,sizeof(char))) == 0)
					DieWithError("receive error");
				if(fileName[i++] == Delimeter) break;
			}
			fileName[i-1] = '\0';
			read(c_socket,&origFileSize,sizeof(int));
			printf("FT<-File Name: %s / File Size : %d\n", fileName, origFileSize);
			/*File ACK send*/
			strcpy(msgType, FileAck);
			printf("FT->%s\n",msgType);
			write(c_socket, msgType, strlen(msgType));
			
			fp = fopen("test/test.txt","w");
			/*receive file contents*/
			while(origFileSize > recvFileSize){
				if((recv_n = read(c_socket, fileBuffer, origFileSize) < 0 ))
					DieWithError("recv failed in ft_mode");
				fileBuffer[origFileSize] = EOF;
				fwrite(fileBuffer, sizeof(char), origFileSize, fp);
				recvFileSize += recv_n;
			}
			printf("%s(%d bytes) uploading success to Server",fileName,recvFileSize);
		}else if(!strcmp(msgType,EchoReq)){
			recv_n = read(c_socket,rcvBuffer,RCVBUFSIZE);
			rcvBuffer[recv_n] = '\0';
			printf("msg<-%s\n",rcvBuffer);
			strcpy(msgType, EchoRep);
			write(c_socket, msgType, strlen(msgType));
			write(c_socket, rcvBuffer, strlen(rcvBuffer));
			printf("msg->%s\n",rcvBuffer);
		}else if(!strcmp(msgType,Quit)){
			break;
		}
		/*
		if(strcmp(rcvBuffer,"FT")==0)
		{
			printf("Client FT Mode\n");
			while(ft_mode(c_socket)>0);	
			continue;
		}
		*/
		/*print to std and record to log*/
		/*
		printf("msg<-%s\n",rcvBuffer);
		record_log(fp, rcvBuffer);

		if(write(c_socket, rcvBuffer, recv_n) < 0)
			DieWithError("send error");
		printf("msg->%s\n",rcvBuffer);
		*/
	}
	close(c_socket);
	pthread_mutex_lock(&mtx);
	for(i=0;i<clnt_cnt;i++)if(c_socket==c_sockets[i])break;;
	for(;i<clnt_cnt;i++)
		c_sockets[i] = c_sockets[i+1];	
	clnt_cnt--;
	pthread_mutex_unlock(&mtx);
	printf("client quit\n");
	return NULL;
}
void record_log(FILE *fp, char * data){
	char *log_file_name = "echo_history.log";

	fp = fopen(log_file_name, "a");
	fprintf(fp, "%s\n", data);
	fclose(fp);
}

int ft_mode(int c_socket){
	int i =0;
	char recvChar;
	char msgType[10];
	char fileName[32];
	char echoString[RCVBUFSIZE];
	char fileBuffer[BUFSIZE];
	int recv_n;
	int origFileSize, recvFileSize;
	FILE *fp;

	while(i < 10){
		if((recvChar = read(c_socket, msgType+i, sizeof(char)) < 0)==0)
			return -1;
		if(msgType[i++] == Delimeter) break;
	}
	printf("FT<-msgType : %s\n",msgType);
	if(!strcmp(msgType,FileUpReq)){
		read(c_socket,fileName,32);
		read(c_socket,&origFileSize,sizeof(int));
		printf("FT<-File Name: %s / File Size : %d\n", fileName, origFileSize);
		/*File ACK send*/
		strcpy(msgType, FileAck);
		printf("FT->%s\n",msgType);
		write(c_socket, msgType, strlen(msgType));
		
		fp = fopen("test/test.txt","w");
		/*receive file contents*/
		while(origFileSize > recvFileSize){
			if((recv_n = read(c_socket, fileBuffer, origFileSize) < 0 ))
				DieWithError("recv failed in ft_mode");
			fileBuffer[origFileSize] = EOF;
			fwrite(fileBuffer, sizeof(char), origFileSize, fp);
			recvFileSize += recv_n;
		}

	}else if(!strcmp(msgType, EchoReq)){
		recv_n = read(c_socket,echoString,RCVBUFSIZE);
		echoString[recv_n] = '\0';
		strcpy(msgType, EchoRep);
		write(c_socket, msgType, strlen(msgType));
		write(c_socket, echoString, strlen(echoString));
	} else
		DieWithError("Bad Request");

	return 1;
}
