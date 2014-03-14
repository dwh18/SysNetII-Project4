#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include "UDPserver.h"


#define BUFFER_SIZE 256
#define MAX_NUM_LISTENER_ALLOWED 10
/*
* The main program that starts up the server.
*/

int main(int argc, char** argv)
{
        int listensockfd, cliLength, recLength;
        struct sockaddr_in cliAddr;
        char message[256];

        //Provides host inforation on screen and creates a datagram server socket
        printHostInfo();
        listensockfd = createSocket();
        if (listensockfd < 0) {
                perror ("cannot create socket");
                exit (1);
        }

        //Continously recieves a response and sends a reply until recieving "<shutdown/>"
        while(1)
        {
                cliLength = sizeof(cliAddr);
                recLength = recvfrom(listensockfd, message, 256, 0, (struct sockaddr *)&cliAddr, (socklen_t *)&cliLength);
                message[recLength] = '\0';
                reqHandler(listensockfd, message, recLength, cliAddr, cliLength);
        }
}
