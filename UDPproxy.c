/*
*   UDPproxy.c
*   Systems and Networks II
*   Project 4
*   By David Hunt and Anthony Martin
*
*/


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
#include "UDPproxy.h"

#define _GNU_SOURCE
#include <string.h>

#define BUFFER_SIZE 256
#define MAX_NUM_LISTENER_ALLOWED 10

/*
* Identifies if the specified message is well-formed or not.
*
* msg - the message to be tested
*
* returns a value greater than 0 if the message is well formed and a value less than zero if it is not
*/
int wellFormed(char* msg)
{
        int length = strlen(msg);
        printf("The message is %s\n", msg);

        if (strcasecmp(msg, "<loadavg/>") == 0)
        {
                if (length != 10)
                        return -1;
                return 1;
        }
        if (strncasecmp(msg, "<echo>", 6) == 0)
        {
                char* endStr = strstr(msg, "</echo>");
                if (endStr == NULL)
                        return -1;
                if (endStr[7] != '\0')
                        return -1;
                return 1;
        }
        if (strcasecmp(msg, "<shutdown/>") == 0)
        {
                if(length != 11)
                        return -1;
                return 1;
        }

        return -1;
}

/*
* Determines the load average and writes it as an XML formatted string into the specified string location.
*
* resp - memory for a text string to store the XML formatted string specifying load average information
*/
void getLoadAvgResp(char* resp)
{
        double resultRuntime[3];

        bzero(resp, BUFFER_SIZE);

        if (getloadavg(resultRuntime, 3) < 0) {
                strcpy(resp, "<error>unable to obtain load average</error>");
                return;
        }

        sprintf(resp, "<replyLoadAvg>%f:%f:%f</replyLoadAvg>", resultRuntime[0], resultRuntime[1], resultRuntime[2]);
}

/*
* Generates a response to a request message in accordance with the given protocol in the project documentation.
* The request message is assumed to be in a correct format.
*
* msg - the message that specifies the request from a client
* resp - memory for a text string to store the response to a request
* sock - socket used by server, can be closed in case shutdown
*/
void getResponse(char* msg, char* resp, int sock)
{
        bzero(resp, BUFFER_SIZE);

        if (strcasecmp(msg, "<loadavg/>") == 0) {
                getLoadAvgResp(resp);
                return;
        }

        if (strncasecmp(msg, "<echo>", 6) == 0) {
                int payLoadLength = strlen(msg)-13;
                strcpy(resp, "<reply>");
                strncpy(&resp[7], &msg[6], payLoadLength);
                strcpy(&resp[7+payLoadLength], "</reply>");
                return;
        }


        if(strcasecmp(msg, "<shutdown/>") == 0)
        {
                fprintf(stderr, "Closing socket & terminating!\n");
                close(sock);

                exit(0);
        }
}

/*
* Writes an error message into the specified character array.
*
* msg - memory for a text string to store the error message
*/
void createError(char* msg)
{
        bzero(msg, BUFFER_SIZE);
        strcpy(msg, "<error>unknown format</error>");
}

/*
* Prints the host name of the machine to the screen and returns the port #.
*/
host * printHostInfo(int argc, char ** argv)
{
        host * recv = NULL;
        if(argc == 3)
        {
                recv = createHost(recv);
                strcpy(recv->hostname, argv[1]);
                recv->port = atoi(argv[2]);
                setAddr(recv);
        }
        else
        {
                fprintf(stderr, "Invalid number of argumnets! Need receiver address and port #\n");
                exit(1);
        }

        // getting host name and use static port binding
        char hostname[128];
        gethostname(hostname, 128);
        fprintf (stderr, "Hostname of Proxy: %s\n", hostname);
        return recv;
}

/*
* Creates a socket and binds it statically. Port information of the bound socket is returned.
*/
int createSocket()
{
        struct sockaddr_in servaddr;
        int listensockfd;

        // creating a streaming server socket
        listensockfd = socket (AF_INET, SOCK_DGRAM, 0);
        if (listensockfd < 0) {
                return -1;
        }

        // build local address and set port to unknown
        memset ((void *) &servaddr, 0, (size_t)sizeof(servaddr));
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(0);

        // bind statically
        bind (listensockfd, (struct sockaddr *) &servaddr, (socklen_t)sizeof(servaddr));

        // retrieve port information
        struct sockaddr_in boundAddr;
        socklen_t sizeAddr = sizeof(boundAddr);
        getsockname(listensockfd, (struct sockaddr *) &boundAddr, &sizeAddr);
        fprintf(stderr, "Port # of the Proxy: %d\n", ntohs(boundAddr.sin_port));

        return listensockfd;
}

/*
* Implements the request handler executed within a thread.
*/
void reqHandler(int sockValue, char * message, int recLength, struct sockaddr_in cliAddr, int cliLength, host * recv)
{       
        int length = 0, tempAddrLen = 0, temp = 0, errno;
        char resdMessage[512];
        emptyBuffer(resdMessage, 512);
        struct sockaddr_in tempAddr;

        temp = sendto(sockValue, message, strlen(message), 0, (struct sockaddr *)recv->hostAddr, sizeof(struct sockaddr_in));
        if(temp <= 0)
        {
                fprintf(stderr, "Could not send message - %d\n", errno);
        }
        printf("Sending to reciever: %s\n", message);
        fflush(stdout);
        length = recvfrom(sockValue, resdMessage, 511, 0, (struct sockaddr *)&tempAddr, (socklen_t *)&tempAddrLen);
        if(length <= 0)
        {
                fprintf(stderr, "Could not receive message - %d\n", errno);
        }
        printf("Received from receiver: %s +with length %d\n", resdMessage, length);
        sendto(sockValue, resdMessage, strlen(resdMessage), 0, (struct sockaddr *)&cliAddr, cliLength);
        printf("Sending: %s\n", resdMessage);
}

host * createHost(host * hst)
{
        hst = (host*)malloc(1+sizeof(host));
        hst->hostname = (char *) malloc(1+20*(sizeof(char)));
        hst->port = 0;
        int i = 0;
        for(i = 0; i < 20; i++)
        {
                hst->hostname[i] = 0;
        }
        hst->hostAddr = (struct sockaddr_in *)malloc(1+sizeof(struct sockaddr_in));
        return hst;
}

void setAddr(host * host1)
{
        struct hostent * htptr;

        if((htptr = gethostbyname(host1->hostname)) == NULL)
        {
                perror("invalid host name\n");
                return ;
        }

        memset((void *)host1->hostAddr, 0, (size_t)sizeof(struct sockaddr_in));
        host1->hostAddr->sin_family = (short)(AF_INET);
        memcpy((void *)&host1->hostAddr->sin_addr, (void *) htptr->h_addr, htptr->h_length);
        host1->hostAddr->sin_port = htons(host1->port);

        host1->hostSize = sizeof(host1->hostAddr); 
}

void emptyBuffer(char * arry, int sizeArry)             //A utility function that empty a buffer of a string
{
        int i;
        for(i = 0; i < sizeArry; i++)
        {
                arry[i] = '\0'; 
        }       
}
