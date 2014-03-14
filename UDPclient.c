/*
 * UDPclient.c
 * Systems and Networks II
 * Project 4
 *
 * This file describes the functions to be implemented by the UDPclient.
 * You may also implement any auxillary functions you deem necessary.
 */
#include <stdio.h> 			/* for printf() and fprintf() */
#include <sys/socket.h>			/* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>			/* for sockaddr_in and inet_addr() */
#include <stdlib.h>			/* for atoi() and exit() */
#include <string.h>			/* for memset() */
#include <unistd.h>			/* for close() */
#include <netinet/in.h>
#include <netdb.h>
#include "UDPclient.h"

/*
 * Creates a datagram socket and connects to a server.
 *
 * serverName - the ip address or hostname of the server given as a string
 * serverPort - the port number of the server
 * dest       - the server's address information; the structure will be initialized with information
 *              on the server (like port, address, and family) in this function call
 *
 * return value - the socket identifier or a negative number indicating the error if a connection could not be established
 */
int createSocket(char * serverName, int serverPort, struct sockaddr_in * dest)
{
	int socketCreation = 0;
	struct hostent * htptr;

	if((htptr = gethostbyname(serverName)) == NULL)
	{
		perror("invalid host name\n");
		return -1;
	}
	//build the destination address
	memset((void *)dest, 0, (size_t)sizeof(struct sockaddr_in));
	dest->sin_family = (short)(AF_INET);
	memcpy((void *)&dest->sin_addr, (void *) htptr->h_addr, htptr->h_length);
	dest->sin_port = htons(serverPort);		
	//establish a connection to a socket
	socketCreation = socket(PF_INET, SOCK_DGRAM, 0);
	return socketCreation;
}

/*
 * Sends a request for service to the server. This is an asynchronous call to the server, 
 * so do not wait for a reply in this function.
 * 
 * sockFD  - the socket identifier
 * request - the request to be sent encoded as a string
 * dest    - the server's address information
 *
 * return   - 0, if no error; otherwise, a negative number indicating the error
 */
int sendRequest(int sockFD, char * request, struct sockaddr_in * dest)
{
	printf("Sending request\n");
	int error = 0;
	error = sendto(sockFD, request, strlen(request), 0, (const struct sockaddr *) dest, sizeof(*dest));
	return error;
}

/*
 * Receives the server's response formatted as an XML text string.
 *
 * sockfd    - the socket identifier
 * response  - the server's response as an XML formatted string to be filled in by this function into the specified string array
 * 
 * return   - 0, if no error; otherwise, a negative number indicating the error
 */
int receiveResponse(int sockFD, char * response)
{
	int error = 0, rdin;
	rdin = recvfrom(sockFD, response, 255, 0, NULL, NULL);
	response[rdin] = '\0';
	return error;
}

/*
 * Prints the response to the screen in a formatted way.
 *
 * response - the server's response as an XML formatted string
 *
 */
void printResponse(char * response)
{
	printf("Response is: %s\n", response);
}

/*
 * Closes the specified socket
 *
 * sockFD - the ID of the socket to be closed
 * 
 * return - 0, if no error; otherwise, a negative number indicating the error
 */
int closeSocket(int sockFD)
{
	int error = 0;
	error = close(sockFD);
	return error;
}

void emptyBuffer(char * arry, int sizeArry)		//A utility function that empty a buffer of a string
{
	int i;
	for(i = 0; i < sizeArry; i++)
	{
		arry[i] = '\0';	
	}	
}

void showASCII(char * str)			//A utility function that shows the ascii number for a string
{
    int i, sentSize;
    sentSize = strlen(str);
    if(sentSize == 0)
    {
    	printf("String is empty\n");
    	return;
    }
    for(i = 0; i < sentSize; i++)
    {
    	printf("%c: ", str[i]);
      	printf("%d ", str[i]);   
    }
    printf("\n");
}

void printArguments(int argc, char ** argv)			//A utility function that shows the arguments entered into the program
{
	printf("Argurment 1 is: %s\n", argv[0]);
	printf("Argurment 2 is: %s\n", argv[1]);
	printf("Argurment 3 is: %s\n", argv[2]);

}
