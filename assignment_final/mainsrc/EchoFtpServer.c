#include "EchoFtpClient.h"
#include <signal.h>

#define FILENAME_SIZE 256
#define END_OF_PROTOCOL "\r\n"
#define COMMAND_MAX_SIZE 32
#define DTP_PORT 9467

typedef struct _FtpClient{
    int clntSock;
    char clntName[32];
}FtpClient;


int clntSock;
int clntDtpSock;
int servSock;
int servDtpSock;
int mode;
struct sockaddr_in servAddr, dtpAddr;

int initializeFtpServer(int port);

int acceptClient(int servSock, struct sockaddr_in *clntAddr, unsigned int *clntLen);
int commandHandle(int clntSock,char *cmd);
void HandleFTPControl(int clntSock);

int main(int argc, char *argv[])
{
    struct sockaddr_in  clntAddr;
    char recvBuffer[RCVBUFSIZE];
    pid_t pid;

    unsigned int clntLen = sizeof(clntAddr);
    if(argc < 2){
        printf("Usage : FTPServer [Port]");
        return 0;
    }
    printf("%s\n",argv[1]);
    servSock = initializeFtpServer(atoi(argv[1]));  

    while(1)
    {
        printf("Wait for new connect... %hd\n",htons(servAddr.sin_port));
        clntSock = acceptClient(servSock, &clntAddr, &clntLen);
        //if((clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clntLen))< 0)
        //   DieWithError("Accept Error");
        pid = fork();
        if (pid < 0 ){  // error
            DieWithError("fork error");
        }
        else if (pid > 0){ // parent
            close(clntSock);
        } else{  // child
            printf("client ip : %s\n", inet_ntoa(clntAddr.sin_addr));
            printf("clinet port: %d\n", ntohs(clntAddr.sin_port));
            HandleFTPControl(clntSock);
        }

        /*recvProtocol(clntSock, recvBuffer, RCVBUFSIZE);
          if(strcmp(recvBuffer, "hello")==0){
          printf("%s\n",recvBuffer);
          sendProtocol(clntSock, "hi");

          HandleFTPControl(clntSock);
          }else{
          DieWithError("connect failed");
          }*/

    }
    return 0;
}

int acceptClient(int servSock, struct sockaddr_in *clntAddr, unsigned int *clntLen)
{
    int clntSock;
    char tempBuffer[32];
    if((clntSock = accept(servSock, (struct sockaddr*)clntAddr, clntLen))< 0)
        DieWithError("Accept Error");

    recvProtocol(clntSock, tempBuffer, 32);
    if(strcmp(tempBuffer, "hello")==0){
       printf("socket %d :%s\n",servSock,tempBuffer);
       sendProtocol(clntSock, "hi");
    }else{
        DieWithError("connect failed");
    }
    return clntSock;
}

int initializeFtpServer(int port)
{
    int servSock;
    //전역변수로 이동 struct sockaddr_in servAddr;

    if((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))< 0)
        DieWithError("socket create failed");
    // dtp 소켓도 열어둔다.
    if((servDtpSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))< 0)
        DieWithError("dtp socket create failed");


    memset(&servAddr, 0, sizeof(servAddr));
    memset(&dtpAddr, 0, sizeof(servAddr));
    /*init addr variable*/
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    dtpAddr.sin_family = AF_INET;
    dtpAddr.sin_port = htons(DTP_PORT);
    dtpAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Bind to the local address*/
    if(bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr))< 0)
        DieWithError("Bind failed");
    /* Mark the socket so it will listen for incoming connections*/
    if(listen(servSock, 32)< 0)
        DieWithError("Listen failed");

    /* Bind to the local address*/
    if(bind(servDtpSock, (struct sockaddr*)&dtpAddr, sizeof(dtpAddr))< 0)
        DieWithError("Bind failed");
    /* Mark the socket so it will listen for incoming connections*/
    if(listen(servDtpSock, 32)< 0)
        DieWithError("Listen failed");

    return servSock;
}
// client로부터 메시지 받아서 해석하여 실행
void HandleFTPControl(int clntSock)
{
    char recvBuffer[RCVBUFSIZE];
    int flag=0;
    while(!flag){
        recvProtocol(clntSock, recvBuffer, RCVBUFSIZE);
        flag = commandHandle(clntSock,recvBuffer);
    }

}

int commandHandle(int clntSock, char *recvBuffer)
{
    char cmd[COMMAND_MAX_SIZE];
    char sendBuffer[RCVBUFSIZE];
    struct sockaddr_in clntAddr;
    unsigned int clntLen;
    if(!strncmp(recvBuffer, "User", 4))
    {
        sendProtocol(clntSock,"Get User name");
    }
    else if(!strncmp(recvBuffer, PRTCL_SYST,4))
    {
        sendProtocol(clntSock,"OS : Linux /  Server ip : 127.0.0.1\r\n");
    }
    else if(!strncmp(recvBuffer, PRTCL_PASSIVE,4))
    {
        // client에게 DTP의 주소 알려줌
        sprintf(sendBuffer,"%s %d",inet_ntoa(dtpAddr.sin_addr),dtpAddr.sin_port);
        sendProtocol(clntSock,sendBuffer);

        // Passive mode 요청이 오면 DTP 서버의 소켓에서  클라이언트 연결을 기다린다.
        clntDtpSock = acceptClient(servDtpSock, &clntAddr, &clntLen);
    }
    else if (!strncmp(recvBuffer,PRTCL_ECHO,4)){
        sendProtocol(clntSock,"ok");
        debug("ECHO\n");
        recvProtocol(clntSock,recvBuffer,BUFSIZE);
        printf("%s",recvBuffer);
        //echo back
        sendProtocol(clntSock,recvBuffer);
    }
    // file upload 받음
    else if(!strncmp(recvBuffer, PRTCL_STOR, 4))
    {
        char fileName[FILENAME_SIZE];
        char filePath[FILENAME_SIZE]; 
        unsigned int fileSize;

        getcwd(filePath, FILENAME_SIZE);    //현재 서버 작업 디렉토리 
        sscanf(recvBuffer, "%*s %s%*c",fileName);

        sprintf(sendBuffer, "226 File Send OK%s",END_OF_PROTOCOL);
        sendProtocol(clntSock,sendBuffer); //sending client PI that file upload is ok

        recvProtocol(clntSock,recvBuffer,RCVBUFSIZE);
        sscanf(recvBuffer,"%*s %u%*c",&fileSize);
        printf("%u",fileSize);
        sprintf(filePath,"%s/%s",filePath,fileName);
        downloadFile(clntDtpSock, fileName, fileSize,1);

        //send to client PI that file upload is completed
        sendProtocol(clntSock,"226 File receiving complete\r\n");
        close(clntDtpSock);
    }
    //file download 요청받음
    else if(!strncmp(recvBuffer, PRTCL_RETR, 4))
    {
        char fileName[FILENAME_SIZE];
        char filePath[FILENAME_SIZE];
        unsigned int fileSize;

        getcwd(filePath, FILENAME_SIZE);    //현재 서버 작업 디렉토리
        
        sscanf(recvBuffer, "%*s %s%*c",fileName);
        sprintf(filePath, "%s/%s",filePath,fileName);
        fileSize = uploadFile(clntDtpSock, fileName, 1);

        sprintf(sendBuffer, "226 File Receive OK(%d)%s",fileSize,END_OF_PROTOCOL); 
        sendProtocol(clntSock, sendBuffer);
        
        sendProtocol(clntSock,"226 File sending complete\r\n");
    }
    else if(!strncmp(recvBuffer, PRTCL_LIST,4))
    {
        FILE *fin = NULL;
        char tempBuffer[80];
        system("ls -l > tmp.txt");
        sprintf(sendBuffer,"150 File transfering.. %s",END_OF_PROTOCOL);
        sendProtocol(clntSock,sendBuffer);
        fin = fopen("tmp.txt","r");
        while(!feof(fin)){
            fgets(tempBuffer,78,fin);
            sprintf(sendBuffer,"%s",tempBuffer);
            sendProtocol(clntDtpSock,sendBuffer);
        }
        fclose(fin);
        close(clntDtpSock);
        sprintf(sendBuffer,"226 File transfer completed ....\r\n");
        sendProtocol(clntSock,sendBuffer);
    }
    else if(!strncmp(recvBuffer, PRTCL_CWD, 3))
    {
        char tempBuffer[80];
        sscanf(recvBuffer, "%*s %s%*c",tempBuffer);
        chdir(tempBuffer);
        system("pwd");
        sprintf(sendBuffer,"Change working directory..%s",END_OF_PROTOCOL);
        sendProtocol(clntSock,sendBuffer);
    }
    else if(!strncmp(recvBuffer,PRTCL_QUIT,4))
    {
        printf("Quit\n");
        sprintf(sendBuffer,"221 Connection closed by the FTP Client %s",END_OF_PROTOCOL);
        sendProtocol(clntSock,sendBuffer);
        //close(clntSock);
        return -1;
    }
    return 0;
}

