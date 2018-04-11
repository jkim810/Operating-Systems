#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int upper_string(char *s);
void sigpipeHandler(int sig);

int accepted = 0;
int rejected = 0;
int matched = 0;

int main (int argc, char **argv) {
	char *dic[500000];
	char buf[BUFSIZ];
	int index = 0;
	size_t len = 0;
	ssize_t rd;

	pid_t cpid;
	int fd[2];

	if (argc < 2){
		fprintf(stderr, "Not enough input arguments. Specify dictionary.\n");
		exit(EXIT_FAILURE);
	}
	
	signal(SIGPIPE, sigpipeHandler);

 	FILE *f;
 	f = fopen(argv[1], "r");

 	if(!f){
 		fprintf(stderr,"Error opening file %s: %s\n", argv[1], strerror(errno));
 		return -1;
 	}

	while(rd = getline(&dic[accepted], &len, f) != -1) {
		if(upper_string(dic[accepted])) {
			accepted++;
		}else{
			rejected++;
		}
	}
	
	if(rd == -1){
		fprintf(stderr, "Error reading from file %s: %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "Accepted %i words, rejected %i words\n", accepted, rejected);
    while (fgets(buf, sizeof buf, stdin) != NULL){   
		index = 0;
		while(index < accepted){
			if(strcmp(buf, dic[index++]) == 0) {
        		printf("%s", buf);
				matched ++;
			}
		}
    }

	fprintf(stderr, "Matched %i words\n", matched);

	index = 0;
	while(index<accepted){
		free(dic[index++]);
	}

	fclose(f);
	exit(EXIT_SUCCESS);
	return 0;
}

int upper_string(char *s) {
	int c = 0;
	while (s[c] != '\0') {
		if (s[c] >= 'a' && s[c] <= 'z') {
			s[c] = s[c] - 32;
		}else{
			if(s[c] < 'A' || s[c] > 'Z') {
				if(s[c] != '\n') return 0;
			}
		}
		c++;
	}
	s[c] = '\0';
	return 1;
}

void sigpipeHandler(int sig){
	fprintf(stderr, "Program interrupted by SIGPIPE, at the point of termination, Matched %i words\n", matched);
	exit(EXIT_FAILURE);
}