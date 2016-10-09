CC = gcc
#CFLAGS = -W 
TARGET1 = TCPEchoClient
TARGET2 = TCPEchoServer
OBJS = TCPEchoClient.o TCPEchoServer.o HandleTCPClient.o DieWithError.o
OBJS1 = TCPEchoClient.o DieWithError.o HandleTCPClient.o
OBJS2 = TCPEchoServer.o HandleTCPClient.o DieWithError.o

.SUFFIXES : .c .o

all : $(TARGET1) $(TARGET2)

$(TARGET1) : $(OBJS1)
	$(CC) -o $@ $(OBJS1)

$(TARGET2) : $(OBJS2)
	$(CC) -o $@ $(OBJS2)

clean :
	rm -f $(OBJS) $(TARGET1) $(TARGET2)
