
#include "FTPClient.h"

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
void rcd(char *cdCmd);
void hash(char *hasCmd);
void quit(char *quitCmd);

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
    { CMD_HASH, hash}, 
    { CMD_QUIT, quit},
};

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

    initializeFtpClient();

    while(1){
        // input user command
        // command 예시
        // ftp> open 127.0.0.1 9999
        // ftp> put file.txt
        if(ip == 0 && port == 0){
            printMessage("ftp>");
            fgets(cmd, COMMAND_MAX_SIZE, stdin);
        }else{
            sprintf(cmd,"open %s %s", ip, port);
            //startCmd = 0;
        }
        // call handler
        commandHandle(cmd);
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
    sprintf(sendBuffer, "SYST%s", END_OF_PROTOCOL);
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

    sprintf(sendBuffer, "PASV%s", END_OF_PROTOCOL);
    // server에 PASV라는 메시지 보냄
    sendProtocol(sock, sendBuffer);
    // 서버로부터 데이터 수신
    recvProtocol(sock, recvBuffer, BUFSIZE - 1);
    printMessage(recvBuffer);
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
// get local directory
void list(char * listCmd){
    system("ls -l");
}

//get remote working directory file list
void rlist(char *rlistCmd){
    int port;
    char ip[16];
    char sendBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE*8];

    debug("rlist");

    // recv server response and parsing
    passiveMode(ip, &port);

    // connect to DTP
    dtpSock = connectServer(ip, ntohs(port));

    // send LIST command to PI server
    sprintf(sendBuffer , "LIST%s", END_OF_PROTOCOL);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE);
    printMessage(recvBuffer);

    // recv file list from DTP
    recvProtocol(dtpSock, recvBuffer, BUFSIZE);
    printf("%s\n",recvBuffer);

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
    sprintf(sendBuffer, "RETR %s%s" , fileName, END_OF_PROTOCOL);
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
    sprintf(sendBuffer, "STOR %s%s", fileName, END_OF_PROTOCOL);
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

    sprintf(sendBuffer, "CWD %s%s", recvBuffer, END_OF_PROTOCOL);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE);
    printMessage(recvBuffer);
}

// ftp client exit
void quit(char *quitCmd)
{
    char sendBuffer[BUFSIZE];
    char recvBuffer[BUFSIZE];
    debug("quit");

    sprintf(sendBuffer, "QUIT%s", END_OF_PROTOCOL);
    sendProtocol(sock, sendBuffer);
    recvProtocol(sock, recvBuffer, BUFSIZE);
    printMessage(recvBuffer);

    close(sock);
    exit(0);
}

//same quit
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
        ip = argv[0];
        port = argv[1];
    }

    startFtpClient(ip,port);
    return 0;
}
