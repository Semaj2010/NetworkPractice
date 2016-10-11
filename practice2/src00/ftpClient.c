#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define  EchoReq 	"EchoReq|" 
#define  FileUpReq	"FileUpReq|" 
#define  EchoRep 	"EchoRep|" 
#define  FileAck 	"FileAck|" 
#define  Delimeter	'|'
#define  Quit       "quit"

#define RCVBUFSIZE 100
#define BUFSIZE 1000

char msgType[10], recvChar;int i=0;
char fileBuffer[BUFSIZE];
char * operation; // ex) client 203.252.164.144 upload test.txt 5000

void DieWithError(char *);
void *do_echo(void *);
void *do_keyboard(void *);
void *do_socket(void *);
int ft_mode_client(int );

char echoString[RCVBUFSIZE];

pthread_t pid[2];

int main(int argc, char *argv[]){

	int c_socket;	
	struct sockaddr_in ftpServAddr;
	char servIP[32];
	unsigned short s_port;
	int thr_id, status;

	if((c_socket=socket(PF_INET, SOCK_STREAM, 0)) < 0)	
		DieWithError("socket error");

	if(argc < 2){
		printf("Server IP:");
		scanf("%s", servIP);
		if(servIP[0]=='0')
			strcpy(servIP,"127.0.0.1\0");
	} else {
		strcpy(servIP,argv[1]);
		//s_port = atoi(argv[1]);
	}
	printf("Server Port:");
	scanf("%hd", &s_port);
	/*construct the server address structure*/
	memset(&ftpServAddr, 0, sizeof(ftpServAddr));
	ftpServAddr.sin_family = AF_INET;
	ftpServAddr.sin_addr.s_addr = inet_addr(servIP);
	ftpServAddr.sin_port = htons(s_port);

	if(connect(c_socket, (struct sockaddr*) &ftpServAddr, sizeof(ftpServAddr)) < 0)
		DieWithError("connect failed");
	printf("Server IP : %s , Port : %hd\n", inet_ntoa(ftpServAddr.sin_addr), ntohs(ftpServAddr.sin_port));
	if(write(c_socket, "hello", 5) != 5)
		DieWithError("send failed");
	if(read(c_socket, echoString,RCVBUFSIZE) < 0)
		DieWithError("receive failed");
	printf("ACK : %s\n", echoString);

	thr_id=pthread_create(&pid[0], NULL, do_keyboard, (void *)c_socket);
	thr_id=pthread_create(&pid[1], NULL, do_socket, (void*)c_socket);

	pthread_join(pid[0], (void **)&status);
	pthread_join(pid[1], (void**)&status);

	close(c_socket);
}
void *do_keyboard(void *data){
	/*키보드로 입력받아서 전송만*/
	int c_socket = (int) data;
	char sBuf[BUFSIZE];
	char echoString[RCVBUFSIZE];
	int n;
	int FT_flag = 0;

	while((n = read(0, echoString, RCVBUFSIZE))>0){
		echoString[n] = '\0';
		printf("msg->%s\n",echoString);
		if(!strncmp(echoString, Quit,4)){
			write(c_socket,Quit,4);
			pthread_kill(pid[1],SIGINT);
			break;
		}else if(!strncmp(echoString, "FT",2)){
			while(ft_mode_client(c_socket)!=0);
			continue;
		}else{
			strcpy(sBuf, EchoReq);
			strncpy(sBuf+strlen(EchoReq),echoString,n);
			if(write(c_socket,sBuf,strlen(EchoReq)+n)<0)
				DieWithError("echo send error");
		}
		memset(echoString,0,RCVBUFSIZE);
	}
}
void echo_mode(){
	
}
void *do_socket(void *data){
	int recv_n;
	char rcvBuffer[RCVBUFSIZE];
	char msgType[10];
	int c_socket = (int)data;
	int i=0;
	while(1){
		i = 0;
		while(i < 10){
			if((recv_n = read(c_socket, msgType+i, RCVBUFSIZE)) < 0)
				DieWithError("receive error");
			if(msgType[i++] == Delimeter) break;
		}
		if(!strcmp(msgType,EchoRep)){
			if((recv_n = read(c_socket, rcvBuffer, RCVBUFSIZE)) > 0 ){
				rcvBuffer[recv_n] = '\0';
				printf("msg<-%s\n",rcvBuffer);
			}
		}
	}
}
void *do_echo(void *data){
	int recv_n;
	char rcvBuffer[RCVBUFSIZE];
	char echoString[RCVBUFSIZE];
	int c_socket = (int) data;

	while(1){
		scanf("%s", echoString);

		if(strcmp(echoString,"/quit")==0){
			close(c_socket);
			printf("Close Connection..\n");
			break;
		}else if(!strcmp(echoString,"FT")){
			printf("Welcome to FT Client!\n");
			while(ft_mode_client(c_socket) != 0);	
			continue;
		} else{


		}
		recv_n = strlen(echoString);
		printf("msg->%s\n",echoString);

		/*send the string to server*/
		if(write(c_socket, echoString, recv_n) != recv_n)
			DieWithError("string send error!");

		if((recv_n = read(c_socket, rcvBuffer, RCVBUFSIZE) )< 0)
			DieWithError("string receive error");
		rcvBuffer[recv_n] = '\0';
		printf("msg<-%s\n",rcvBuffer);
	}
	return NULL;
}

int ft_mode_client(int c_socket){
	char filename[32];
	FILE *fp;
	int file_name_len;
	int fileSize;
	char msgType[10];

	printf("filename to put to server : ");
	scanf("%s",filename);

	if(!strcmp(filename,"quit")){
		write(c_socket,"quit",4);
		return 0;
	}else{
		fp = fopen(filename, "r");
		fseek(fp,0,SEEK_END);
		fileSize = ftell(fp);
		fseek(fp,0,SEEK_SET);

		file_name_len = strlen(filename);	
		filename[file_name_len] = Delimeter;

		strcpy(msgType,FileUpReq);

		if(write(c_socket,msgType,strlen(msgType))<0)
			DieWithError("msgType send error");
		if(write(c_socket,filename,file_name_len+1)<0)
			DieWithError("filename send error");
		if(write(c_socket,&fileSize,sizeof(int))<0)
			DieWithError("filesize send error");
		read(c_socket,msgType,10);
		printf("msgType:%s\n",msgType);
		if(!(strcmp(msgType,FileAck))){
			fread(fileBuffer,sizeof(char),fileSize,0);
			//fileBuffer[fileSize] = '\0';
			if(write(c_socket,fileBuffer,fileSize)<0)
				DieWithError("File send error");
		}
		fclose(fp);
	}
	return 1;
}
