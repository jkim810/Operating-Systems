#include "fifo.h"

int proc, reader, status;
int writers = 8;
int length = 2048;

int main(){
	struct fifo *f;
	if((f = (struct fifo * ) mmap(NULL, sizeof (struct fifo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED){
		fprintf(stderr, "Syscall mmap for fifo failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	fifo_init(f);
	printf("Beginning test with %d writers, %d items each\n", writers, length);
	pid_t pid[writers];
	for(int i = 0; i < writers; i++){
		if((pid[i] = fork()) < 0){
			fprintf(stderr, "Unable to fork id#%d: %s\n",i, strerror(errno));
			exit(EXIT_FAILURE);
		}
		if(pid[i] == 0){
			proc = i;
			unsigned long wr_buf[length];
			for(int j = 0; j < length; j++){
				wr_buf[j] = getpid()*10000 + j;
				fifo_wr(f, wr_buf[j]);
			}
			printf("Writer %d completed\n", i);
			exit(EXIT_SUCCESS);
		}
	}

	if((reader = fork()) < 0){
		fprintf(stderr, "Unable to fork: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(reader == 0){
		unsigned long rd_buf[writers*length];
		int nread = writers*length;
		for(int i = 0; i < nread; i++){
			rd_buf[i] = fifo_rd(f);
		}
		printf("All streams done\n");
	}
	
	printf("Waiting for writer children to die\n");
	for(int i = 0; i < writers + 1; i++){
		while(wait(&status)>0);
	}

	return 0;
}
