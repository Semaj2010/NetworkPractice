
#include "EchoFtpClient.h"

#define COMMAND_MAX_SIZE 1024
#define FILENAME_SIZE 256
#define END_OF_PROTOCOL "\r\n"

void initializeFtpClient();
void startFtpClient(char *ip, char *port);
void commandHandle(char *cmd);
void defaultHandler(char *cmd);
int modeCheck(const char *option);
void printMessage(char *msg);

void openCon(char *cmd);
void rlist(char *rlistCmd);
void list(char *list);
void get(char *getCmd);
void put(char *putCmd);
void rcd(char *rcdCmd);
void cd(char *cdCmd);
void hash(char *hasCmd);
void quit(char *quitCmd);
void echo(char *echoCmd);

unsigned int downloadFile(int sock, char *filePath, unsigned int fileSize, int hashFlag);
unsigned int uploadFile(int sock, char *filePath, int hashFlag);

void debug(char *);

typedef struct _FtpCmdHandler{
    char cmd[5];
    void (*handler)(char* arg);
} FtpCmdHandler;

//Map Ftp Command to Handler
FtpCmdHandler ftpCmdHandler[] = {
    { CMD_OPEN, openCon},
    { CMD_RLIST, rlist},
    { CMD_LIST, list},
    { CMD_PUT, put},
    { CMD_GET, get},
    { CMD_RCD, rcd},
    { CMD_CD, cd},
    { CMD_HASH, hash}, 
    { CMD_QUIT, quit},
};
// global variable
//sock - PI socket, dtpSock - DTP socket
int sock, dtpSock;
int state;
int mode;
int hashFlag;

int connectServer(char *serverIP, short port) {
    int sock;
    struct sockaddr_in servAddr;
    char recvBuffer[32];

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        DieWithError("sock failed");
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(serverIP);
    servAddr.sin_port = htons(port);

    if(connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1)
    {
        DieWithError("connect failed");
    }
    sendProtocol(sock,"hello");
    recvProtocol(sock,recvBuffer,32);
    printf("%s\n",recvBuffer);
    return sock;
}


int modeCheck(const char *option){
    if(!strcmp(option, "-d")){
        //debug mode
        mode = MODE_DEBUG;
    }else{
        mode = MODE_NORMAL;
    }
    return mode;
}

//initialize ftp client
void initializeFtpClient(){
    hashFlag = 1;
    //state = INITIAL_STATE;
    debug("initialized");
}

// ftp client start
void startFtpClient(char *ip, char *port)
{
    char cmd[COMMAND_MAX_SIZE];
    char *prompt = "input>";
    int ftp_flag = 0;

    initializeFtpClient();

    while(1){
        // input user command
        // command 예시
        // input> open 127.0.0.1 9999  # connect server
        // input> hello     # echo
        // input> ftp       # mode change
        // ftp> put file.txt    # send file
        // ftp> rls         # get server's file list in current directory
        // ftp> rcd         # change server's directory
        if(ip == 0 && port == 0){
            printMessage(prompt);
            fgets(cmd, COMMAND_MAX_SIZE, stdin);
        }else {
            sprintf(cmd,"open %s %s", ip, port);
            ip = 0;
            port = 0;
            //startCmd = 0;
        }
        /*------------위에서 받은 input에 대해 처리하는 부분 --------------*/
        // check cmd input
        if (!strncmp(cmd,"ftp",3)){
            ftp_flag = 1;
            prompt = "ftp>";
        }else if((ftp_flag == 1) && !strncmp(cmd,"quit",4)){
            ftp_flag = 0; 
            prompt = "input>";
        } else if(ftp_flag == 1){
            // call handler
            commandHandle(cmd);
        } else if(!strncmp(cmd, "open", 4)){
            openCon(cmd);
        } else if(!strncmp(cmd, "quit",4)){
            quit(0);
        } else{
            echo(cmd);
        }
    }
}

// map command to handler
void commandHandle(char *cmd)
{
    int i;
    int numCmd = sizeof(ftpCmdHandler)/sizeof(FtpCmdHandler);
    for (i = 0; i < numCmd; i++){
        if(!strncmp(cmd, ftpCmdHandler[i].cmd, strlen(ftpCmdHandler[i].cmd))){
            (*(ftpCmdHandler[i].handler))(cmd);
            break;
        }
    }
}

void defaultHandler(char *cmd)
{ 
    printf("default handler: %s\n", cmd);
}

// ftp Server connect
void openCon(char* openCmd){
    char serverIP[16], serverPort[16];
    char cmd[BUFSIZE];
    char sendBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE];

    sscanf(openCmd, "%*s %s %s%*c", serverIP,serverPort);
    debug(serverIP);

    // connect to server
    sock = connectServer(serverIP, atoi(serverPort));
    //recvProtocol(sock, recvBuffer, BUFSIZE-1);

    // send user name
    printf("Name: ");
    fgets(cmd, COMMAND_MAX_SIZE, stdin);
    sprintf(sendBuffer, "User %s", cmd);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE - 1);

    // get server os information
    sprintf(sendBuffer, "%s%s",PRTCL_SYST, END_OF_PROTOCOL);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE - 1);
    printMessage(recvBuffer);
}

// send EPSV or PASS to server
void passiveMode(char* ip, int* port){
    char sendBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE];
    int host0, host1, host2, host3;
    int port0, port1;

    sprintf(sendBuffer, "%s%s", PRTCL_PASSIVE,END_OF_PROTOCOL);
    // server에 PASV라는 메시지 보냄
    sendProtocol(sock, sendBuffer);
    // 서버로부터 ACK 데이터 수신
    recvProtocol(sock, recvBuffer, BUFSIZE - 1);
    //printMessage(recvBuffer);
    //서버로 부터 받은 데이터는 아이피와 포트번호. 앞의 네개 숫자는 아이피, 뒤의 두개는 포트번호
    //sscanf(strchr(recvBuffer, '(') + 1, "%d,%d,%d,%d,%d,%d", &host0, &host1, &host2, &host3, &port0, &port1);
    // 복잡해서 그냥 내가 임의로 보냄 127.0.0.1 9467 이런식
    sscanf(recvBuffer, "%d.%d.%d.%d %d", &host0,&host1,&host2,&host3,&port0);
    sprintf(ip, "%d.%d.%d.%d", host0, host1, host2, host3);
    //*port = port0*256 + port1;
    *port = port0;
    debug(ip);
    printf("dtp port: %d\n", ntohs(*port));
}

void echo(char *echoCmd){
    char sendBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE];

    // send ECHO command to PI server
    sprintf(sendBuffer , "%s%s",PRTCL_ECHO, END_OF_PROTOCOL);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE);
    //printMessage(recvBuffer);

    // send ECHO message
    sendProtocol(sock,echoCmd);
    //echo back
    recvProtocol(sock,recvBuffer,RCVBUFSIZE);
    printf("%s",recvBuffer);
}
// get local directory
void list(char * listCmd){
    system("ls -l");
}

//get remote working directory file list
void rlist(char *rlistCmd){
    int port;
    char ip[16];
    char sendBuffer[BUFSIZE];
    char tempBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE*4];
    int len = 1;

    debug("rlist");

    // recv server response and parsing
    passiveMode(ip, &port);

    // connect to DTP
    dtpSock = connectServer(ip, ntohs(port));

    // send LIST command to PI server
    sprintf(sendBuffer , "%s%s",PRTCL_LIST, END_OF_PROTOCOL);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE);
    printMessage(recvBuffer);

    // recv file list from DTP
    while(len>0){
        len = recvProtocol(dtpSock, recvBuffer, BUFSIZE); 
        printf("%s",recvBuffer);
    }

    // recv complete message from PI server
    recvProtocol(sock, recvBuffer , BUFSIZE);
    printMessage(recvBuffer);

    close(dtpSock);

}

// file download
// ftp> get test.txt
void get (char *getCmd) {
    int port;
    unsigned int fileSize;
    char ip[16], filePath[FILENAME_SIZE], fileName[50];
    char sendBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE];

    // get local current working directory
    getcwd(filePath, FILENAME_SIZE);
    sscanf(getCmd, "%*s %s%*c", fileName);

    debug("get");
    printf("fileName: %s\n", fileName);
    printf("filePath: %s\n", filePath);

    passiveMode(ip, &port);

    // connect to DTP
    dtpSock = connectServer(ip, ntohs(port));

    //request server for trasnfer start - RETR fileName
    sprintf(sendBuffer, "%s %s%s" ,PRTCL_RETR, fileName, END_OF_PROTOCOL);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE);    // 응답으로 파일 사이즈 받음
    printMessage(recvBuffer);

    // extract fileSize
    sscanf(strchr(recvBuffer, '(')+1, "%u", &fileSize);
    printf("fileSize: %u\n", fileSize);

    // download file from DTP
    downloadFile(dtpSock, fileName, fileSize, hashFlag);

    // recv complete message from PI server
    recvProtocol(sock, recvBuffer, BUFSIZE);
    printMessage(recvBuffer);

    close(dtpSock);
}

void put(char *putCmd)
{
    int port;
    unsigned int fileSize;
    char ip[16], filePath[FILENAME_SIZE], fileName[50];
    char sendBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE];

    sscanf(putCmd, "%*s %s%*c", fileName);

    // get local current working directory
    getcwd(filePath, FILENAME_SIZE);
    sscanf(putCmd, "%*s %s%*c", fileName);
    sprintf(filePath, "%s/%s", filePath, fileName);

    debug("put");
    debug(filePath);

    passiveMode(ip, &port);

    //connect to DTP
    dtpSock = connectServer(ip, ntohs(port));

    //request server for transfer start - STOR fileName
    sprintf(sendBuffer, "%s %s%s",PRTCL_STOR, fileName, END_OF_PROTOCOL);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE);
    printMessage(recvBuffer);

    // file upload to DTP
    fileSize = uploadFile(dtpSock, fileName, hashFlag);
    sprintf(sendBuffer, "SIZE %d%s",fileSize,END_OF_PROTOCOL);
    sendProtocol(sock,sendBuffer);

    close(dtpSock);

    // recv complete message from PI server
    recvProtocol(sock, recvBuffer, BUFSIZE);
    printMessage(recvBuffer);
}

// change remote working directory
void rcd(char *rcdCmd)
{
    char sendBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE];
    debug("rcd");

    sscanf(rcdCmd, "%*s %s%*c", recvBuffer);
    debug(recvBuffer);

    sprintf(sendBuffer, "%s %s%s",PRTCL_CWD, recvBuffer, END_OF_PROTOCOL);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE);
    printMessage(recvBuffer);
}

void cd(char *cdCmd)
{
    char tempBuffer[100];
    sscanf(cdCmd, "%*s %s%*c", tempBuffer);
    chdir(tempBuffer);
    system("pwd");
}
// ftp client exit
void quit(char *quitCmd)
{
    char sendBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE];
    debug("quit");

    if (!sock)
        exit(0);
    sprintf(sendBuffer, "%s%s",PRTCL_QUIT ,END_OF_PROTOCOL);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE);
    printMessage(recvBuffer);

    close(sock);
    exit(0);
}

//same as quit
void bye(char *byeCmd)
{
    quit(0);
}
// get remote working directory
void pwd(char *pwdCmd)
{
    char sendBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE];
    debug("pwd");

    sprintf(sendBuffer, "PWD%s", END_OF_PROTOCOL);
    sendProtocol(sock,sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE);
    printMessage(recvBuffer);
}
// hash option on/off
void hash(char *hashCmd)
{
    debug("hash");
    hashFlag = !hashFlag;

    if(hashFlag == 0)
    {
        printMessage("hash off");
    }else{
        printMessage("hash on");
    }
}
void printMessage(char *msg){
    printf("%s", msg);
}

int main(int argc, char *argv[])
{
    /*
     * 클라이언트 프로세스 플로우
     * FTP Server IP와 Port 입력. 접속
     *
     * */
    char *ip;
    char *port;
    //mode =MODE_DEBUG;
    if(argc < 3){
        ip = NULL;
        port = NULL;
    }else{
        ip = argv[1];
        port = argv[2];
    }
    printf("사용법\nftp>open [IP] [PORT]\nftp>put test.txt\nftp>get test.txt\nftp>rls\nftp>rcd path/to/somewhere\n");
    startFtpClient(ip,port);
    return 0;
}
