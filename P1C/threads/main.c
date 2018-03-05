#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "msgstruct.h"


#define SERVER_QUEUE_NAME   "/sp-example-server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

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

int client(struct msg* message)
{
    sync_printf("Client Thread: Active\n");
    char client_queue_name [64];
    mqd_t qd_server, qd_client;   // queue descriptors


    // create the client queue for receiving messages from server
    sprintf(client_queue_name, "/sp-example-client-%d", getpid ());

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    //open client message queue from server
    if ((qd_client = mq_open (client_queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        sync_printf("ERROR Client: mq_open (client)\n");
        return 1;
    }

    //open server message queue from client
    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        sync_printf("ERROR Client: mq_open (server)\n");
        return 1;
    }

    char in_buffer [MSG_BUFFER_SIZE];

        sync_printf("Client: Sending Message to server\n");
	// send header message to server (contains queue name for response)
	if (mq_send (qd_server, client_queue_name, strlen (client_queue_name), 0) == -1) {
	    sync_printf("ERROR Client: Not able to send message to server\n");
	    //continue;
	}

	//send string from message struct
	if (mq_send (qd_server, message->str, message->strlen, 0) == -1) {
	    sync_printf("ERROR Client: Not able to send message to server\n");
	    //continue;
	}

	// receive response from server
	if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
	    sync_printf("ERROR Client: mq_receive\n");
	    return 1;
	}
	// display string received from server
	sync_printf("Client: String received from server: %s\n", in_buffer);

	// receive response from server
	if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
	    sync_printf("ERROR Client: mq_receive\n");
	    return 1;
	}
	// display LED settings received from server
	sync_printf("Client: LED Settings received from server: %#04x\n", *((char*)in_buffer));


    if (mq_close (qd_client) == -1) {
        sync_printf("ERROR Client: mq_close\n");
        return 1;
    }

    if (mq_unlink (client_queue_name) == -1) {
        sync_printf("ERROR Client: mq_unlink\n");
        return 1;
    }
    sync_printf("Client: Exiting\n");

    return 0;
}

int server(struct msg* message)
{
    mqd_t qd_server, qd_client;   // queue descriptors

    sync_printf("Server Thread: Active\n");

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        sync_printf("ERROR Server: mq_open (server)\n");
        return 1;
    }
    
    //while (1) {
    char header [MSG_BUFFER_SIZE];
    char in_buffer [MSG_BUFFER_SIZE];
   // char out_buffer [MSG_BUFFER_SIZE];


        // get the header message
        if (mq_receive (qd_server, header, MSG_BUFFER_SIZE, NULL) == -1) {
            sync_printf("ERROR Server: mq_receive\n");
            return 1;
        }

        sync_printf("Server: Header message received.\n");

        // get the string
        if (mq_receive (qd_server, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            sync_printf("ERROR Server: mq_receive\n");
            return 1;
        }

        sync_printf("Server: Message string received: %s\n", in_buffer);

	//open client based on header message address

        if ((qd_client = mq_open (header, O_WRONLY)) == 1) {
            sync_printf("ERROR Server: Not able to open client queue\n");
            return 1;
        }

       // sprintf(out_buffer, "%ld", token_number);

        //send reply message to client

        if (mq_send (qd_client, message->str, message->strlen, 0) == -1) {
            sync_printf("ERROR Server: Not able to send message to client\n");
            return 1;
        }

        sync_printf("Server: response sent to client.\n");
    //}

    sync_printf("Server: Exiting\n");
    return 0;
}

int main(void)
{
        pid_t   childpid;

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
		client(message1);
                exit(0);
        }
        else
        {
		server(message2);
        }
        return(0);
}
