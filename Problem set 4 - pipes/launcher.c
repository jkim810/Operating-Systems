#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void closePipes(int *pipe1, int *pipe2);
void waitfork(pid_t cpid, int stat);

int main(int argc, char ** argv){
	pid_t gen, search, page;
	int fd1[2], fd2[2];
	int status;

	char *arggen[] = {"./wordgen", argv[1], NULL};
	char *argsearch[] = {"./wordsearch", "wordlist_small.txt", NULL};
	char *argpage[] = {"./pager", NULL};

	if(pipe(fd1) == -1 || pipe(fd2) == -1){
		fprintf(stderr, "Error creating pipe: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	switch(gen = fork()){
		case -1:
			fprintf(stderr, "Error creating fork for wordgen: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		case 0:
			if(dup2(fd1[1], STDOUT_FILENO) < 0){
				fprintf(stderr, "Error dup2 in first pipe connection: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			closePipes(fd1, fd2);
			execvp(arggen[0], arggen);
			fprintf(stderr, "Error occured executing wordgen: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		default:
			switch(search = fork()){
				case -1:
					fprintf(stderr, "Error creating fork for wordsearch: %s\n", strerror(errno));
					exit(EXIT_FAILURE);
				case 0:
					if(dup2(fd1[0], STDIN_FILENO) < 0 || dup2(fd2[1], STDOUT_FILENO) <0){
						fprintf(stderr, "Error dup2 in second pipe connection: %s\n", strerror(errno));
						exit(EXIT_FAILURE);
					}
					closePipes(fd1,fd2);
					execvp(argsearch[0], argsearch);
					fprintf(stderr, "Error occured executing wordsearch: %s\n", strerror(errno));
					exit(EXIT_FAILURE);
				default:
					switch(page = fork()){
						case -1:
							fprintf(stderr, "Error creating fork for pager: %s\n", strerror(errno));
							exit(EXIT_FAILURE);
						case 0:
							if(dup2(fd2[0], STDIN_FILENO) < 0){
								fprintf(stderr, "Error dup2 in pager: %s\n", strerror(errno));
								exit(EXIT_FAILURE);
							}
							closePipes(fd1,fd2);
							execvp(argpage[0], argpage);
							fprintf(stderr, "Error occured executing pager: %s\n", strerror(errno));
							exit(EXIT_FAILURE);
					}
			}
	}
	closePipes(fd1, fd2);

	waitfork(page, status);
	waitfork(search, status);
	waitfork(gen, status);
	
	exit(EXIT_SUCCESS);
}


void closePipes(int *pipe1, int *pipe2){
	if(close(pipe1[0]) < 0 || close(pipe1[1]) < 0)
	{
		fprintf(stderr, "Error closing pipe 1: %s", strerror(errno));
		exit(EXIT_FAILURE);	
	}
	if(close(pipe2[0]) < 0 || close(pipe2[1]) < 0)
	{
		fprintf(stderr, "Error closing pipe 2: %s", strerror(errno));
		exit(EXIT_FAILURE);	
	}
}

void waitfork(pid_t cpid, int stat){
	do{
		if(waitpid(cpid, &stat, 0) == -1){
			fprintf(stderr, "Error waiting cpid %i: %s\n",cpid, strerror(errno));
			exit(EXIT_FAILURE);	
		}
		if(WIFEXITED(stat)){
			printf("Child %i exited with %i\n", cpid, stat);
		}else if(WIFSIGNALED(stat)){
			printf("Child %i killed by signal %d\n", cpid, WTERMSIG(stat));
		}
	}while(!WIFEXITED(stat) && !WIFSIGNALED(stat));
}
