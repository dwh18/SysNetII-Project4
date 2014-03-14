#Student Names: Anthony Martin & David Hunt
#Project 4

CC = gcc
CFLAGS = -Wall -lpthread

TARGET1 = server
TARGET2 = client
TARGET3 = proxy

all: $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1): UDPserver.c UDPserver.h ServerMain.c
	$(CC) $(CFLAGS) UDPserver.c UDPserver.h ServerMain.c -o $(TARGET1)
	
$(TARGET2): UDPclient.c UDPclient.h UDPmain.c 
	$(CC) $(CFLAGS) UDPclient.c UDPmain.c UDPclient.h -o $(TARGET2)
	
$(TARGET3): UDPproxy.c UDPproxy.h ProxyMain.c
	$(CC) $(CFLAGS) UDPproxy.c UDPproxy.h ProxyMain.c -o $(TARGET3)

.PHONY: clean

clean:
	rm -f *.o
	rm $(TARGET1)
	rm $(TARGET2)
	rm $(TARGET3)
