CC = gcc
CFLAGS = -W -g
TARGET1 = FTPClient
TARGET2 = FTPServer
OBJS = FTPClient.o FTPServer.o DieWithError.o
OBJS1 = FTPClient.o DieWithError.o FTPClient.h
OBJS2 = FTPServer.o DieWithError.o FTPClient.h

.SUFFIXES : .c .o

all : $(TARGET1) $(TARGET2)

$(TARGET1) : $(OBJS1)
	$(CC)  -o $@ $(OBJS1)

$(TARGET2) : $(OBJS2)
	$(CC)  -o $@ $(OBJS2)



clean :
	rm -f $(OBJS) $(TARGET1) $(TARGET2)
