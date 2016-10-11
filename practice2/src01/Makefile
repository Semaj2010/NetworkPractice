CC = gcc
CFLAGS = -W -g
TARGET1 = ftClient
TARGET2 = ftServer
OBJS = ftClient.o ftServer.o DieWithError.o
OBJS1 = ftClient.o DieWithError.o 
OBJS2 = ftServer.o DieWithError.o

.SUFFIXES : .c .o

all : $(TARGET1) $(TARGET2)

$(TARGET1) : $(OBJS1)
	$(CC)  -o $@ $(OBJS1)

$(TARGET2) : $(OBJS2)
	$(CC)  -o $@ $(OBJS2)

clean :
	rm -f $(OBJS) $(TARGET1) $(TARGET2)
