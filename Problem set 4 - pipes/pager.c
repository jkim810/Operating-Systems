#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

int main(){
	int numline = 0;
	char buf[BUFSIZ];
	char cmd[BUFSIZ];
	size_t len = 0;
	ssize_t fd;

	while (fgets(buf, sizeof buf, stdin) != NULL)
	{
		numline++;
		printf("%s", buf);
		if(numline == 23){
			printf("---Press RETURN for more---\n");
			cmd[0] = '\0'; //flushes cmd
			fd = open("/dev/tty", O_RDONLY);
			read(fd, cmd, sizeof cmd);
			close(fd);
			if(cmd[0] == 'Q' || cmd[0] == 'q'){
				printf("*** Pager terminated by Q command ***\n");
				exit(EXIT_SUCCESS);
			}
			if(cmd[0] == '\0'){
				printf("*** Pager terminated by EOF ***\n");
				exit(EXIT_SUCCESS);
			}

			numline = 0;
			
		}
	}
	return 0;
}