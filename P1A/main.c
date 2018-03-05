#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "msgstruct.h"

int main(void)
{
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
                printf("Child received string: %s, %d bytes long\n", c_readbuffer, nbytes);
		nbytes = read(fd2[0], c_readbuffer, 1);
		printf("Child received LED settings: %#04x, 1 byte long\n",  *((char*)c_readbuffer));

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
                printf("Parent received string: %s, %d bytes long\n", p_readbuffer, nbytes);
		nbytes = read(fd1[0], p_readbuffer, 1);
		printf("Parent received LED settings: %#04x, 1 byte long\n",  *((char*)p_readbuffer));
        }
        return(0);
}
