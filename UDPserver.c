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
#include "UDPserver.h"

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
void printHostInfo()
{
        // getting host name and use static port binding
        char hostname[128];
        gethostname(hostname, 128);
        fprintf (stderr, "Hostname: %s\n", hostname);
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
                fprintf(stderr, "Error creating socket\n");
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
        fprintf(stderr, "Port #: %d\n", ntohs(boundAddr.sin_port));

        return listensockfd;
}

/*
* Implements the request handler executed within a thread.
*/
void reqHandler(int sockValue, char * message, int recLength, struct sockaddr_in cliAddr, int cliLength)
{
        int sockfd = sockValue;
        char resp[BUFFER_SIZE];

        if (wellFormed(message) < 0) {
                createError(resp);
        }
        else {
                getResponse(message, resp, sockValue);
        }
        
        recLength = strlen(resp);
        sendto(sockfd, resp, recLength, 0, (struct sockaddr *)&cliAddr, cliLength);
        printf("Sending: %s +with length %d\n", resp, strlen(resp));
}
