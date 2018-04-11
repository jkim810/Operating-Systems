#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

char* removeOperand(int start, char* str);

int main(int argc, char *argv[]) {
	char buf[1024];
	char* line = NULL;
	char* argument = NULL;
	char* cmd = NULL;
	int op;
	int f, r;
	struct timeval rstart, rfinish, rdiff;

	if(argc > 1) {
		if((op = open(argv[1], O_RDONLY)) < 0) {
			fprintf(stderr, "Failed to open file '%s' for read: %s\n", argv[1], strerror(errno));
			return 1;
		}
		if((r = read(op, buf, 1023)) < 0) {
			fprintf(stderr, "Failed to read file '%s': %s\n", argv[1], strerror(errno));
			return 1;
		}
	} else {
		r = read(STDIN_FILENO, buf, sizeof(buf) - 1);
	}
	buf[r-1] = '\0';
	
	line = strtok(buf,"\n");
	while(line != NULL) {
		if(line[0] != '#') {
			gettimeofday(&rstart, NULL);	
			getrusage(RUSAGE_CHILDREN, &rustart);	
			getrusage(RUSAGE_CHILDREN, &rsstart);
			if((f = fork()) >= 0) {
				if(f == 0) {//child	
					char args[1024];
					char* redir = NULL;
					argument = strtok(line, " ");
					cmd = argument;
					int argCount = 0;
					int fd;
					while(argument != NULL) {
						if(argument[0] == '<') {
							redir = removeOperand(1,argument);
							if((fd = open(redir, O_RDONLY)) < 0) {
								fprintf(stderr, "Failed to open file '%s' for reading : %s\n", redir, strerror(errno));
								return 1;
							}
							dup2(fd, STDIN_FILENO);
							close(fd);
						} else if(argument[0] == '>') {
							if(argument[1] == '>') {
								redir = removeOperand(2,argument);
								if((fd = open(redir, O_WRONLY|O_CREAT|O_APPEND,0666)) < 0) {
									fprintf(stderr, "Failed to open file '%s' for reading : %s\n", redir, strerror(errno));
									return 1;
								}
								dup2(fd, STDOUT_FILENO);
								close(fd);
							} else {
								redir = removeOperand(1,argument);
								if((fd = open(redir, O_WRONLY|O_CREAT|O_TRUNC,0666)) < 0) {
									fprintf(stderr, "Failed to open file '%s' for reading : %s\n", redir, strerror(errno));
									return 1;
								}
								dup2(fd, STDOUT_FILENO);
								close(fd);
							}
						} else if(argument[0] == '2' && argument[1] == '>') {
							if(argument[2] == '>') {
								redir = removeOperand(3,argument);
								if((fd = open(redir, O_WRONLY|O_CREAT|O_APPEND,0666)) < 0) {
									fprintf(stderr, "Failed to open file '%s' for reading : %s\n", redir, strerror(errno));
									return 1;
								}
								dup2(fd, STDERR_FILENO);
								close(fd);
							} else {
								redir = removeOperand(2,argument);
								if((fd = open(redir, O_WRONLY|O_CREAT|O_TRUNC,0666)) < 0) {
									fprintf(stderr, "Failed to open file '%s' for reading : %s\n", redir, strerror(errno));
									return 1;
								}
								dup2(fd, STDERR_FILENO);
								close(fd);
							}
						} else {
							strcat(args, argument);
							strcat(args, " ");
							argCount++;
						}
						argument = strtok(NULL, " ");
					}
					
					char* argv2[argCount+1];
					argument = strtok(args, " ");
					int i = 0;
					while(argument != NULL) {
						argv2[i] = argument;
						argument = strtok(NULL, " ");
						i++;
					}		
					argv2[argCount] = NULL;
					
					if(execvp(cmd, argv2) < 0) {
						fprintf(stderr, "Failed to run command '%s' with arguments ", cmd);
						for(int j = 1;j < (sizeof(argv2)/sizeof(argv2[0]))-1;j++) {
							fprintf(stderr, "'%s' ", argv2[j]);
						}
						fprintf(stderr, ": %s\n", strerror(errno));
						return 1;
					}			
					
					return 0;			
				} else {
					int zombie;
					wait(&zombie);
					printf("Command returned with return code %d\n", zombie);
				}
			} else {
				fprintf(stderr, "Failed to fork : %s\n", strerror(errno));
				return 1;
			}
			gettimeofday(&rfinish, NULL);
			getrusage(RUSAGE_CHILDREN, &rufinish);
			getrusage(RUSAGE_CHILDREN, &rsfinish);
			timersub(&rfinish, &rstart, &rdiff)
			printf("consuming %lu.%03lu real seconds, %lu.%03lu user, %lu.%03lu system\n",real,ureal,user,ureal,system,usystem);
		}
		line = strtok(NULL,"\n");
	}
	printf("end of file read\n");
	return 0;
}

char* removeOperand(int start, char* str) {
	size_t length = strlen(str);
	char newStr[length-start+1];
	for(int i = start;i < length;i++) {
		newStr[i-start] = str[i];
	}
	newStr[length-start] = '\0';
	char* rtn = newStr;
	return rtn;
}
