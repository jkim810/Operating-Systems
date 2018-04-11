#include "fifo.h"

int procno, i, k, reader, n, w;
int numwriters = 5;
int writelen = MYFIFO_BUFSIZ;

int main(){
  fifo *fif;
	fif = (fifo * )mmap (NULL, sizeof (fifo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if(fif < 0){
	fprintf(stderr, "CRITICAL ERROR: Unable to mmap ANONYMOUS page for fifo: %s\n", strerror(errno));
	exit(1);
  }
  fifo_init(fif);
  printf("Beginning test with %d writers, %d items each\n", numwriters, writelen);
  pid_t pid[numwriters];
  for(i = 0; i < numwriters; i++){
	if((pid[i] = fork()) < 0){
  	//FORK ERROR
    	fprintf(stderr, "NON-CRITICAL ERROR: Unable to fork id#%d: %s\n",i, strerror(errno));
    	return(1);
	}
	if(pid[i] == 0){
  	//CHILD
  	procno = i;
  	unsigned long writebuf[writelen];
  	for(k = 0; k < writelen; k++){
    	writebuf[k] = getpid()*10000 + k;
    	fifo_wr(fif, writebuf[k]);
  	}
  	printf("Writer %d completed\n", i);
  	exit(0);
	}
  }

  if((reader = fork()) < 0){
	//FORK ERROR
    	fprintf(stderr, "NON-CRITICAL ERROR: Unable to fork: %s\n", strerror(errno));
    	return(1);
  }
  if(reader == 0){
	//CHILD
	procno = numwriters;
	unsigned long readbuf[numwriters*writelen];
	int nread = numwriters*writelen;
	for(w = 0; w < nread; w++){
  	readbuf[w] = fifo_rd(fif);
	}
	printf("All streams done\n");
  }

  for(n = 0; n < numwriters + 1; n++){
	printf("Waiting for children to die\n");
	wait(0);
  }

  return(0);
}
