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
​ ​*​ ​@brief​ ​Shared Memory IPC Example
​ ​*
​ ​*​ ​This​ ​is an example of how shared memory (mmap) can be used for IPC
 * (Inter-process communication)
​ ​*
​ ​*​ ​@author​ ​Zach Farmer
​ ​*​ ​@date​ ​Mar 3 2018
​ ​*​ ​@version​ ​69
​ ​*
​ ​*/

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
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

int main() {
	pthread_mutex_init(&printf_mutex, NULL);
	printf("Shared Memory IPC Example\n\n");

	pid_t   childpid;

	struct msg* message1 = malloc(sizeof(struct msg));

	message1->str = "From child to parent";
	message1->strlen = 20;
	message1->LED = 0x10;

	struct msg* message2 = malloc(sizeof(struct msg));

	message2->str = "From parent to child";
	message2->strlen = 20;
	message2->LED = 0x01;
	
	void* shmem = mmap(NULL, 128, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);

	memcpy(shmem, message1, 128);
	struct msg* rx = shmem;
	printf("Parent thread writes string: %s\n",  rx->str);
	printf("Parent thread writes string length: %d\n",  rx->strlen);
	printf("Parent thread writes LED settings: %#04x\n", rx->LED);
	childpid = fork();

	if (childpid == 0) {
		struct msg* rx = shmem;
		printf("Child thread reads string: %s\n", rx->str);
		printf("Child thread reads string length: %d\n",  rx->strlen);
		printf("Child thread reads LED settings: %#04x\n", rx->LED);
		memcpy(shmem, message2, 128);
		printf("Child thread writes string: %s\n",  rx->str);
		printf("Child thread writes string length: %d\n",  rx->strlen);
		printf("Child thread writes LED settings: %#04x\n", rx->LED);

	} else {
		sleep(1);
		struct msg* rx = shmem;
		printf("Parent thread reads string: %s\n", rx->str);
		printf("Parent thread reads string length: %d\n",  rx->strlen);
		printf("Parent thread reads LED settings: %#04x\n", rx->LED);
	}

	//unmap shared memory
	munmap(shmem, 128);
}
