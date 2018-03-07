/*****************************************************************************
​ ​*​ ​Copyright​ ​(C)​ ​2018 ​by​ Zach Farmer
​ ​*
​ ​*​ ​Redistribution,​ ​modification​ ​or​ ​use​ ​of​ ​this​ ​software​ ​in​ ​source​ ​or​ ​binary
​ ​*​ ​forms​ is permitted under the Zach Literally Could Not Care Less If You 
 * Paid Him To License and GNU GPL.
 *
 * ​Zach Farmer ​is not liable for any misuse of this material.
​ ​*
*****************************************************************************/
/**
​ ​*​ ​@file​ ​main.c
​ ​*​ ​@brief​ ​Sockets IPC Example
​ ​*
​ ​*​ ​This​ ​is an example of how sockets can be used for IPC
 * (Inter-process communication) and networking
 * (if you set up ports and/or port forwarding correctly)
​ ​*
​ ​*​ ​@author​ ​Zach Farmer
​ ​*​ ​@date​ ​Mar 3 2018
​ ​*​ ​@version​ ​69
​ ​*
​ ​*/

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "msgstruct.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
  

#define PORT 55171
#define HOSTNAME "ubuntu"

pthread_mutex_t printf_mutex;
pthread_mutex_t log_mutex;

/**
​ ​*​ ​@brief​ ​Synchronous encapsulator for printf
​ ​*
​ ​*​ ​Mutexes printf for asynchronous call protection
 * among multiple threads
​ ​*
​ ​*​ ​@param​ ​format print formatting
 * @param ... variadic arguments for print (char *, char, etc)
​ *
​ ​*​ ​@return​ void
​ ​*/
void sync_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    pthread_mutex_lock(&printf_mutex);
    vprintf(format, args);
    pthread_mutex_unlock(&printf_mutex);

    va_end(args);
}

/**
​ ​*​ ​@brief​ ​client-side socket process function
​ ​*
​ ​*​ ​sends a sample message struct to a server at a given location
​ ​*
​ ​*​ ​@param​ ​portno port number
 * @param hostname the hostname of the server
​ *
​ ​*​ ​@return​ 0 if successful
​ ​*/
int client(struct msg* message, int portno, char* hostname)
{
    int sock_ret, err_ret;
    struct hostent *server;
    struct sockaddr_in server_addr;


    char buffer[256];

    sync_printf("Client Thread: Active\n");

    sock_ret = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ret < 0) 
        sync_printf("Client: ERROR opening socket");

    server = gethostbyname(hostname);

    if (server == NULL) {
        sync_printf("Client: ERROR, host does not seem to exist\n");
        return 1;
    }
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&server_addr.sin_addr.s_addr,
         server->h_length);
    server_addr.sin_port = htons(portno);
    if (connect(sock_ret,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) 
        sync_printf("Client: ERROR connecting");

    err_ret = write(sock_ret,message->str, message->strlen);
    if (err_ret < 0) 
         sync_printf("Client: ERROR writing to socket");

    err_ret = write(sock_ret,&(message->LED), 1);

    if (err_ret < 0) 
         sync_printf("Client: ERROR writing to socket");

    err_ret = read(sock_ret,buffer,message->strlen);
    if (err_ret < 0) 
         sync_printf("Client: ERROR reading from socket");
    sync_printf("Client: %s\n",buffer);

    err_ret = read(sock_ret,buffer,message->strlen);
    if (err_ret < 0) 
         sync_printf("Client: ERROR reading from socket");

    sync_printf("Client: LED Settings received from server: %#04x\n", *((char*)buffer));
   
    sync_printf("Client: Exiting\n");

    return 0;
}

/**
​ ​*​ ​@brief​ ​server-side socket process function
​ ​*
​ ​*​ ​sends a sample message struct to a given client at a given location after receiving message from that client
​ ​*
​ ​*​ ​@param​ ​message message sent to client
​ ​*​ ​@param​ ​portno port number
​ *
​ ​*​ ​@return​ 0 if successful
​ ​*/
int server(struct msg* message, int portno)
{
     int sock_ret, newsock_ret, err_ret;;
     unsigned int cli_length;
     char buffer[256];
     struct sockaddr_in server_addr, cli_addr;

     sync_printf("Server Thread: Active\n");

     sock_ret = socket(AF_INET, SOCK_STREAM, 0);
     if (sock_ret < 0) 
        sync_printf("Server: ERROR opening socket");
     bzero((char *) &server_addr, sizeof(server_addr));
     server_addr.sin_family = AF_INET;
     server_addr.sin_addr.s_addr = INADDR_ANY;
     server_addr.sin_port = htons(portno);

     if (bind(sock_ret, (struct sockaddr *) &server_addr,
              sizeof(server_addr)) < 0) 
              sync_printf("Server: ERROR on socket bind");

     listen(sock_ret,5);
     cli_length = sizeof(cli_addr);

     newsock_ret = accept(sock_ret, (struct sockaddr *) &cli_addr, &cli_length);
     if (newsock_ret < 0) 
          sync_printf("Server: ERROR on accept");

     err_ret = read(newsock_ret,buffer,message->strlen);

     if (err_ret < 0) sync_printf("Server: ERROR reading from socket");

     sync_printf("Server: Received string message: %s\n",buffer);

     err_ret = read(newsock_ret,buffer,message->strlen);

     if (err_ret < 0) sync_printf("Server: ERROR reading from socket");

     sync_printf("Server: LED Settings received from client: %#04x\n", *((char*)buffer));

     err_ret = write(newsock_ret,message->str,message->strlen);

     if (err_ret < 0) sync_printf("Server: ERROR writing to socket");

     err_ret = write(newsock_ret,&(message->LED),1);

     if (err_ret < 0) sync_printf("Server: ERROR writing to socket");

     sync_printf("Server: Exiting\n");

     return 0;
}

int main(void)
{
	pthread_mutex_init(&printf_mutex, NULL);
   	pthread_mutex_init(&log_mutex, NULL);
        pid_t   childpid;

    	sync_printf("Sockets IPC Example\n\n");

	struct msg* message1 = malloc(sizeof(struct msg));

	message1->str = "From child to parent";
	message1->strlen = 20;
	message1->LED = 0x10;

	struct msg* message2 = malloc(sizeof(struct msg));

	message2->str = "From parent to child";
	message2->strlen = 20;
	message2->LED = 0x01;

        
        if((childpid = fork()) == -1)
        {
                perror("fork\n");
                exit(1);
        }

        if(childpid == 0)
        {
		client(message1, PORT, HOSTNAME);
                exit(0);
        }
        else
        {
		server(message2, PORT);
        }
        return(0);
}
