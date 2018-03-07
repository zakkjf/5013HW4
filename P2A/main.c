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
​ ​*​ ​@brief​ ​Pipes IPC Example
​ ​*
​ ​*​ ​This​ ​is an example of how pipes can be used for IPC
 * (Inter-process communication)
​ ​*
​ ​*​ ​@author​ ​Zach Farmer
​ ​*​ ​@date​ ​Mar 3 2018
​ ​*​ ​@version​ ​69
​ ​*
​ ​*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "msgstruct.h"
#include <pthread.h>
#include <stdarg.h>

pthread_mutex_t printf_mutex;

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

int main(void)
{
	pthread_mutex_init(&printf_mutex, NULL);
	sync_printf("Pipes IPC Example\n\n");

        int     fd1[2], fd2[2], nbytes;
        pid_t   childpid;
        char    p_readbuffer[21], c_readbuffer[21];

	struct msg* message1 = malloc(sizeof(struct msg));

	message1->str = "From child to parent";
	message1->strlen = 20;
	message1->LED = 0x10;

	struct msg* message2 = malloc(sizeof(struct msg));

	message2->str = "From parent to child";
	message2->strlen = 20;
	message2->LED = 0x01;

        pipe(fd1);
	pipe(fd2);
        
        if((childpid = fork()) == -1)
        {
                perror("fork");
                exit(1);
        }

        if(childpid == 0)
        {
                /* Child process closes up input side of pipe1 */
                close(fd1[0]);
 		/* Child process closes up output side of pipe2 */
		close(fd2[1]);

                /* Send "string" through the output side of pipe1 */
                write(fd1[1], message1->str, strlen(message1->str));
                write(fd1[1], &(message1->LED), 1);

   		/* Read in a string from the pipe2 */
                nbytes = read(fd2[0], c_readbuffer, message1->strlen);
                sync_printf("Child received string: %s, %d bytes long\n", c_readbuffer, nbytes);
		nbytes = read(fd2[0], c_readbuffer, 1);
		sync_printf("Child received LED settings: %#04x, 1 byte long\n",  *((char*)c_readbuffer));

                exit(0);
        }
        else
        {
                /* Parent process closes up output side of pipe1 */
                close(fd1[1]);
                /* Parent process closes up input side of pipe2 */
                close(fd2[0]);

                /* Send "string" through the output side of pipe2 */
                write(fd2[1], message2->str, strlen(message2->str));
                write(fd2[1], &(message2->LED), 1);

                /* Read in a string from the pipe1 */
                nbytes = read(fd1[0], p_readbuffer, message2->strlen);
                sync_printf("Parent received string: %s, %d bytes long\n", p_readbuffer, nbytes);
		nbytes = read(fd1[0], p_readbuffer, 1);
		sync_printf("Parent received LED settings: %#04x, 1 byte long\n",  *((char*)p_readbuffer));
        }
        return(0);
}
