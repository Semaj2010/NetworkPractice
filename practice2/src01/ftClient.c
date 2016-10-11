#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define  EchoReq 	"EchoReq|" 
#define  FileUpReq	"FileUpReq|" 
#define  EchoRep 	"EchoRep|" 
#define  FileAck 	"FileAck|" 
#define  Delimeter	'|'
#define  Quit       "quit|"

#define RCVBUFSIZE 100
#define BUFSIZE   1000

void DieWithError(char *errorMessage);
int ft_mode_client(int c_socket);
char msgType[15];
int main(int argc, char *argv[]){

	int c_socket;
	struct sockaddr_in servAddr;
	unsigned short s_port;
	char servIP[32];
	char sBuf[RCVBUFSIZE];
	char input[RCVBUFSIZE];
	unsigned int s_len;
	int recv_n;

	memset(sBuf,0x00,RCVBUFSIZE);

	if((c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket failed");
	if(argc > 2){
		s_port = (unsigned short)atoi(argv[2]);
		strcpy(servIP,argv[1]);
	}else{
		printf("Server IP : ");
		scanf("%s", servIP);
		printf("Server Port : ");
		scanf("%hd",&s_port);
	}
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(servIP);
	servAddr.sin_port = htons(s_port);

	if(connect(c_socket, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0 )
		DieWithError("connect failed");
	printf("Server IP : %s Port %hd\n", inet_ntoa(servAddr.sin_addr), ntohs(servAddr.sin_port));

	/*its for 3-way handshake*/
	if(send(c_socket, "hello", 5, 0) != 5)
		DieWithError("send() sent a different number of bytes than expected");
	if(recv(c_socket,sBuf,RCVBUFSIZE,0)<0)
		DieWithError("recv() failed");
	printf("%s\n",sBuf);

	while(1){
		memset(input,0x0,RCVBUFSIZE);
		printf("msg->");		
		scanf("%s", input);
		if(!strncmp(input,Quit,4)){
			write(c_socket,Quit,strlen(Quit));
			printf("Close connection ...\n");
			break;
		}else if(!strcmp(input,"FT")){
			printf("Welcome to FT Client!\n");
			printf("example : please type the \"new.txt\" file to Server. Server will get the file to test/test.txt\n");
			while(ft_mode_client(c_socket) != 0);
		} else{
			if(write(c_socket,EchoReq,strlen(EchoReq))<0)
				DieWithError("echo send error");
			if((recv_n = write(c_socket,input,RCVBUFSIZE))<0)
				DieWithError("echo send error");
			if((read(c_socket,input,strlen(EchoRep)))<0)
				DieWithError("receive error");
			if((recv_n = read(c_socket, sBuf, recv_n))<0)
				DieWithError("receive error");
			sBuf[recv_n] = '\0';
			printf("msg<-%s\n",sBuf);

		}

	}
	close(c_socket);

	return 0;	
}
int ft_mode_client(int c_socket){
	char fileName[RCVBUFSIZE];
	FILE *fp;
	int file_name_len;
	int fileSize;
	char fileBuffer[BUFSIZE];
	int recv_n;
	int i=0;

	printf("filename to put to server : ");
	scanf("%s",fileName);

	if(!strcmp(fileName,"quit")){
		return 0;
	}else{
		fp = fopen(fileName, "r");
		fseek(fp,0,SEEK_END);
		fileSize = ftell(fp);
		fseek(fp,0,SEEK_SET);

		file_name_len = strlen(fileName);	
		printf("File Name : %s / File Size : %d\n",fileName,fileSize);

		//strcpy(msgType,FileUpReq);

		if(write(c_socket,FileUpReq,strlen(FileUpReq))<0)
			DieWithError("msgType send error");
		if(write(c_socket,fileName,file_name_len)<0)
			DieWithError("fileName send error");
		if(write(c_socket,"|",sizeof(char))<0)
			DieWithError("Delimeter send error");
		if(write(c_socket,&fileSize,sizeof(int))<0)
			DieWithError("filesize send error");
		i=0;
		while(i<10){
			read(c_socket, msgType+i, sizeof(char));
			if(msgType[i++]==Delimeter) break;
		}
		msgType[i] = '\0';
		printf("msgType:%s\n",msgType);
		if(!(strcmp(msgType,FileAck))){
			fread(fileBuffer,sizeof(char),fileSize,fp);
			//	printf("%s\n",fileBuffer);
			//fileBuffer[fileSize] = '\0';
			if((recv_n = write(c_socket,fileBuffer,fileSize))!=fileSize)
				DieWithError("File send error");
			printf("recv_n : %d\n",recv_n);
			printf("%s File sending....\n",fileName);
		}
		fclose(fp);
	}
	return 1;
}
