#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "ftCommand.h"

void DieWithError(char *errorMessage);
void recvProtocol(int, char *, int);
void sendProtocol(int, char*);
char msgType[15];

extern int mode;

void recvProtocol(int sock, char *recvBuffer, int bufferSize)
{
    int recvLen;

    if((recvLen = recv(sock, recvBuffer, bufferSize-1, 0))<= 0){
        DieWithError("recv failed");
    }
    recvBuffer[recvLen] = '\0';
    if(MODE_DEBUG == mode){
        printf("recv: %s", recvBuffer);
    }
}

void sendProtocol(int sock, char *protocol){
    int protocol_len = strlen(protocol);
    if(send(sock, protocol, protocol_len, 0) != protocol_len){
        DieWithError("send failed");
        exit(1);
    }
    if(MODE_DEBUG == mode){
        printf("send: %s", protocol);
    }
}
// download file function
unsigned int downloadFile(int sock, char *filePath, unsigned int fileSize, int hashFlag){
    char readBuffer[BUFSIZE];
    unsigned int readBytes, totalBytes, numHash;

    FILE *fd = fopen(filePath,"w");

    numHash = 0;
    totalBytes = numHash;
    while(totalBytes < fileSize){
        if((readBytes = read(sock, readBuffer, BUFSIZE))<= 0){
            fclose(fd);
            return totalBytes;
        }
        fwrite(readBuffer, sizeof(char),BUFSIZE, fd);
        totalBytes += readBytes;
        /*hashFlag=1이면 다운로드 상황 출력*/
        if(hashFlag){
            if((totalBytes/BUFSIZE) > numHash) {
                numHash++;
                printf("##");
            }
        }
    }
    fclose(fd);
    printf("\n");
    return totalBytes;
}
// upload file function
unsigned int uploadFile(int sock, char *filePath, int hashFlag){
    char readBuffer[BUFSIZE];
    unsigned int readBytes, totalBytes, numHash;

    FILE *fd = fopen(filePath,"r");
    if(feof(fd)){
        printf("File Error ! \n");
        exit(1);
    }
        
    totalBytes = numHash = 0;
    printf("\nProgrss=>");
    while(!feof(fd)){
        readBytes = fread(readBuffer, sizeof(char), BUFSIZE, fd);
        //printf("%d\n",readBytes);
        readBytes = write(sock, readBuffer, readBytes);
        //printf("%d\n",readBytes);
        totalBytes += readBytes;

        /*hashFlag=1이면 업로드 상황 출력*/
        if(hashFlag){
            if((totalBytes/BUFSIZE) > numHash){
                numHash++;
                printf("##");
            }
        }
    }
    fclose(fd);
    printf("\n");

    return totalBytes;
}




void debug(char *msg) {
	if (mode == MODE_DEBUG) {
		printf("[debug] : %s \n", msg);
	}
}

