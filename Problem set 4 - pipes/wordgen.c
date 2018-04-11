#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

static char *rand_string(char *str, size_t size);

int main(int argc, char ** argv){
	char *line;
	int num = 0;
	size_t size = 5;
	srand(time(NULL));

	if(argv[1] != NULL) num = atoi(argv[1]);

	if(!num){
		fprintf(stderr, "No input argument. Generating words in an endless loop.\n");
		while(1){
			line = rand_string(line, size - 2);
			printf("%s\n", line);
		}
	}else{
		int i = 0;
		while(i++ < num){
			line = rand_string(line, size - 2);
			printf("%s\n", line);
		}
		fprintf(stderr, "Finished generating %i candidate words\n", num);
	
	}
	return 0;
}

static char *rand_string(char *str, size_t size){
	int n = 0, max = 0;
	time_t t;
	max = max + rand() % ((int) size) + 3;
	while (n < max) {
	    char c = (rand () % 26 + 65);
	    str[n++] = c;
	}
	str[n++] = '\0';
	
    return str;
}