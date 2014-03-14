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

typedef struct host 
{
	char * hostname;
	int port;
	struct sockaddr_in * hostAddr;
	int hostSize;
}host;

/*
* Identifies if the specified message is well-formed or not.
*
* msg - the message to be tested
*
* returns a value greater than 0 if the message is well formed and a value less than zero if it is not
*/
int wellFormed(char* msg);

/*
* Determines the load average and writes it as an XML formatted string into the specified string location.
*
* resp - memory for a text string to store the XML formatted string specifying load average information
*/
void getLoadAvgResp(char* resp);

/*
* Generates a response to a request message in accordance with the given protocol in the project documentation.
* The request message is assumed to be in a correct format.
*
* msg - the message that specifies the request from a client
* resp - memory for a text string to store the response to a request
* sock - socket used by server, can be closed in case shutdown
*/
void getResponse(char* msg, char* resp, int sock);

/*
* Writes an error message into the specified character array.
*
* msg - memory for a text string to store the error message
*/
void createError(char* msg);

/*
* Prints the host name of the machine to the screen and returns a the host information about the receiver.
*/
host * printHostInfo(int argc, char ** argv);

/*
* Creates a socket and binds it statically. Port information of the bound socket is returned.
*/
int createSocket();

/*
* Implements the request handler executed within a thread.
*/
void reqHandler(int sockValue, char * message, int recLength, struct sockaddr_in cliAddr, int cliLength, host * recv);

host * createHost(host * hst);

void setAddr(host * host1);

void emptyBuffer(char * arry, int sizeArry);
