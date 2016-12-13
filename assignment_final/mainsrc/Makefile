CC = gcc
CFLAGS = -W -g
TARGET1 = EchoFtpClient
TARGET2 = EchoFtpServer
OBJS = EchoFtpClient.o EchoFtpServer.o DieWithError.o
OBJS1 = EchoFtpClient.o DieWithError.o EchoFtpClient.h ftCommand.h
OBJS2 = EchoFtpServer.o DieWithError.o EchoFtpClient.h ftCommand.h

.SUFFIXES : .c .o

all : $(TARGET1) $(TARGET2)

$(TARGET1) : $(OBJS1)
	$(CC)  -o $@ $(OBJS1)

$(TARGET2) : $(OBJS2)
	$(CC)  -o $@ $(OBJS2)



clean :
	rm -f $(OBJS) $(TARGET1) $(TARGET2)
