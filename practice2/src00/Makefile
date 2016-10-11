CC = gcc
CFLAGS = -W -g
TARGET1 = ftpClient
TARGET2 = ftpServer
OBJS = ftpClient.o ftpServer.o DieWithError.o
OBJS1 = ftpClient.o DieWithError.o 
OBJS2 = ftpServer.o DieWithError.o

.SUFFIXES : .c .o

all : $(TARGET1) $(TARGET2)

$(TARGET1) : $(OBJS1)
	$(CC)  -o $@ $(OBJS1) -lpthread

$(TARGET2) : $(OBJS2)
	$(CC)  -o $@ $(OBJS2) -lpthread

clean :
	rm -f $(OBJS) $(TARGET1) $(TARGET2)
