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

void error(char *msg)
{
    sync_printf(msg);
    exit(0);
}

int client(struct msg* message, int portno, char* hostname)
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    sync_printf("Client Thread: Active\n");
/*
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
*/
    //portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("Client: ERROR opening socket");
    //server = gethostbyname(argv[1]);
    server = gethostbyname(hostname);

    if (server == NULL) {
        sync_printf("Client: ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("Client: ERROR connecting");

    n = write(sockfd,message->str, message->strlen);
    if (n < 0) 
         error("Client: ERROR writing to socket");

    n = write(sockfd,&(message->LED), 1);

    if (n < 0) 
         error("Client: ERROR writing to socket");

    n = read(sockfd,buffer,message->strlen);
    if (n < 0) 
         error("Client: ERROR reading from socket");
    sync_printf("Client: %s\n",buffer);

    n = read(sockfd,buffer,message->strlen);
    if (n < 0) 
         error("Client: ERROR reading from socket");

    sync_printf("Client: LED Settings received from server: %#04x\n", *((char*)buffer));
   
    sync_printf("Client: Exiting\n");

    return 0;
}

int server(struct msg* message, int portno)
{
     int sockfd, newsockfd;
     unsigned int clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sync_printf("Server Thread: Active\n");

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("Server: ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("Server: ERROR on binding");

     listen(sockfd,5);
     clilen = sizeof(cli_addr);

     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
          error("Server: ERROR on accept");

     //bzero(buffer,256);

     n = read(newsockfd,buffer,message->strlen);

     if (n < 0) error("Server: ERROR reading from socket");

     sync_printf("Server: Received string message: %s\n",buffer);

     n = read(newsockfd,buffer,message->strlen);

     if (n < 0) error("Server: ERROR reading from socket");

     sync_printf("Server: LED Settings received from client: %#04x\n", *((char*)buffer));

     n = write(newsockfd,message->str,message->strlen);

     if (n < 0) error("Server: ERROR writing to socket");

     n = write(newsockfd,&(message->LED),1);

     if (n < 0) error("Server: ERROR writing to socket");

     sync_printf("Server: Exiting\n");

     return 0;
}

int main(void)
{
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
