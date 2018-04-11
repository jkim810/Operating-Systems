#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

jmp_buf env;

void signalHandler(int sig){
	fprintf(stderr, "Signal #%d, \"%s\" received!\n", sig, strsignal(sig));
	if(sig!=SIGCHLD && sig!=SIGCONT && sig!=SIGWINCH) exit(sig);
	else fprintf(stderr, "Non-critical Signal Ignored\n");
}

void errorPrompt(){
	fprintf(stderr, "Usage: mmap [123456] (test_no)\n");		
	exit(255);
}

int createFile(size_t length){
	char *execstring;
	size_t sz;
	sz = snprintf(NULL, 0, "dd if=/dev/urandom of=random_text.txt bs=%zu count=1", length);
	if((execstring = (char*) malloc(sz+1)) == NULL){
		fprintf(stderr, "Error malloc failed: %s\n", strerror(errno));
		exit(255);
	}
	snprintf(execstring, sz+1, "dd if=/dev/urandom of=random_text.txt bs=%zu count=1", length);
	if(system(execstring) == 01){
		fprintf(stderr, "Failed to create random text file: %s\n",strerror(errno));
		exit(255);
	}
	int fd = open("random_text.txt", O_RDWR, 0666);
	if(fd == -1){
		fprintf(stderr, "Failed to open random text file: %s\n", strerror(errno));
		exit(255);
	}
	return fd;
}

int test1(){
	char *map;
	size_t length = 8195;
	int fd = createFile(length);
	if((map = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mapping file to memory: %s\n", strerror(errno));
		exit(255);
	}
	fprintf(stderr,"map[3]==\'%c\'\n", map[3]);
	fprintf(stderr,"writing a \'%c\'\n", map[4]);
	if(setjmp(env) == 0){
		map[0] = '0';
	}
	if(munmap(map,length) == -1){
		fprintf(stderr, "Error unmapping file to memory: %s\n", strerror(errno));
		exit(255);
	}
	if(close(fd) == -1){
		fprintf(stderr, "Error closing file descriptor: %s\n", strerror(errno));
		exit(255);
	}
	exit(0);
}

int test23(int flags){
	char *map, *buffer;
	size_t length = 8195;
	int fd = createFile(length);
	if((map = mmap(NULL, length, PROT_READ|PROT_WRITE, flags, fd, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mapping file to memory: %s\n", strerror(errno));
		exit(255);
	}
	char *test_str = "CCCCC";
	int i = 0, offset = 25, wbytes = strlen(test_str);
	fprintf(stderr,"Altering %d bytes to offset %d from %p stored in memory map.\n", wbytes, offset, map);
	for(i = 0; i < wbytes; i++) map[offset+i] = test_str[i];

	if(lseek(fd,offset,SEEK_SET) == -1){
		fprintf(stderr, "Error lseek on %d: %s\n", fd, strerror(errno));
		exit(255);
	}
	if((buffer = malloc(wbytes+1)) == NULL){
		fprintf(stderr, "Error in malloc: %s\n", strerror(errno));
		exit(255);
	}
	if(read(fd, buffer, wbytes) == -1){
		fprintf(stderr, "Error reading from %d: %s\n", fd, strerror(errno));
		exit(255);
	}
	int ans = strncmp(buffer, test_str, wbytes);
	printf("Update to the mapped memory with %s %s\n", flags==MAP_SHARED?"MAP_SHARED":"MAP_PRIVATE", ans? "not visible": "is visible.");
	if(munmap(map,length) == -1){
		fprintf(stderr, "Error unmapping file to memory: %s\n", strerror(errno));
		exit(255);
	}
	if(close(fd) == -1){
		fprintf(stderr, "Error closing file descriptor: %s\n", strerror(errno));
		exit(255);
	}
	ans? exit(0):exit(1);
}


int test45(int testno){
	char *map;
	size_t length = 6144;
	int fd = createFile(length);
	int result;
	struct stat sb;
	if(fstat(fd, &sb) == -1){
		fprintf(stderr, "Error stat on file %d: %s\n", fd, strerror(errno));
		exit(255);
	}
	int orig_size = sb.st_size;
	if((map = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mapping file to memory: %s\n", strerror(errno));
		exit(255);
	}
	char *test_str4 = "XYZ";
	int i =0, offset = length, wbytes = strlen(test_str4);
	for(i=0; i < wbytes; i++) map[offset+i] = test_str4[i];
	if(fstat(fd, &sb) == -1){
		fprintf(stderr, "Error stat on file %d: %s\n", fd, strerror(errno));
		exit(255);
	}
	int new_size = sb.st_size;
	result = new_size != orig_size;
	if(testno == 5){
		char *buffer, *test_str5 = "ABC";
		int i = 0, bytes5 = 16, offset = length, wbytes = strlen(test_str5);
		lseek(fd,bytes5, SEEK_END);
		if(write(fd, test_str5, strlen(test_str5)) == -1){
			fprintf(stderr, "Error writing %s to file %d: %s\n", test_str5, fd, strerror(errno));
			exit(255);
		}
		if(lseek(fd,offset,SEEK_SET) == -1){
			fprintf(stderr, "Error lseek on %d: %s'n", fd, strerror(errno));
			exit(255);
		}
		if((buffer = malloc(bytes5 + wbytes + 1)) == NULL){
			fprintf(stderr, "Error malloc failed: %s\n", strerror(errno));
			exit(255);
		}
		if(read(fd, buffer, bytes5+wbytes*2) == -1){
			fprintf(stderr, "Error reading from %d: %s\n", fd, strerror(errno));
			exit(255);
		}
		result = !strncmp(buffer, test_str4, strlen(test_str4));
	}

	if(munmap(map, length) == -1){
		fprintf(stderr, "Error unmapping file to memory: %s\n", strerror(errno));
		exit(255);
	}
	if(close(fd) == -1){
		fprintf(stderr, "Error closing file: %s\n", strerror(errno));
		exit(255);
	}

	if(testno == 4){
		if(result) {
			printf("Size of the file from stat changed\n");
			exit(0);
		}else{
			printf("Size of the file from stat did not change\n");
			exit(1);
		}	
	}else{
		if(result){
			printf("Byte written is visible\n");
			exit(0);
		}else{
			printf("Byte written is not visible\n");
			exit(1);
		}
	}
}

int test6(){
	char *map;
	size_t length = 20;
	int fd = createFile(length);
	if((map = mmap(NULL, 8192, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mapping file to memory: %s\n", strerror(errno));
		exit(255);
	}
	fprintf(stderr, "Attempting to access memory from first page.\n");
	if(sigsetjmp(env,1) == 0) printf("Reading a byte from offset 2048: %d\n", map[2048]);
	fprintf(stderr, "Attempting to access memory from second page.\n");
	if(sigsetjmp(env,1) == 0) printf("Reading a byte from offset 4096: %d\n", map[4096]);
	if(munmap(map, length) == -1){
		fprintf(stderr, "Error unmapping file to memory: %s\n", strerror(errno));
		exit(255);
	}
	if(close(fd) == -1){
		fprintf(stderr, "Error closing file: %s\n", strerror(errno));
		exit(255);
	}
	exit(0);
}

int main(int argc, char *argv[]){
	for(int i = 0; i < 32; i++) signal(i, signalHandler);
	if(argc != 2) errorPrompt();
	int test_no = atoi(argv[1]);
	if(!(test_no >=1 && test_no <=6)) errorPrompt();
	switch(test_no){
		case 1:
			printf("Executing Test #1 wrtie to r/o mmap\n");
			test1();
			break;
		case 2:
			printf("Executing Test #2 write to MAP_SHARED\n");
			test23(MAP_SHARED);
			break;
		case 3:
			printf("Executing Test #3 write to MAP_PRIVATE\n");
			test23(MAP_PRIVATE);
			break;
		case 4:
			printf("Executing Test #4 writing beyond the edge\n");
			test45(4);
			break;
		case 5:
			printf("Executing Test #5 writing into a hole\n");
			test45(5);
			break;
		case 6:
			printf("Executing Test #6 reading beyond the edge\n");
			test6();
			break;
		default:
			errorPrompt();
			break;
	}
}