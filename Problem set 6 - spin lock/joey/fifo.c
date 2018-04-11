#include "fifo.h"

int i, z;

void fifo_init(struct fifo *fifo){
  cv *rdmap = NULL;
  cv *wrmap = NULL;
  rdmap = (cv *)mmap(NULL, sizeof(cv), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if(rdmap < 0){
	fprintf(stderr, "CRITICAL ERROR: Unable to mmap ANONYMOUS file for read: %s\n", strerror(errno));
	exit(1);
  }
  wrmap = (cv *)mmap(NULL, sizeof(cv), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if(wrmap < 0){
	fprintf(stderr, "CRITICAL ERROR: Unable to mmap ANONYMOUS file for read: %s\n", strerror(errno));
	exit(1);
  }
  fifo->rd = *rdmap;
  cv_init(&fifo->rd);
  fifo->nextread = 0;
  fifo->wr = *wrmap;
  cv_init(&fifo->wr);
  fifo->nextwrite = 0;
  fifo->state = 0;
  fifo->fifolock.primitive_lock = 0;
}

void fifo_wr(struct fifo *fifo, unsigned long d){
  spin_lock(&fifo->fifolock);

  while(fifo->state >= MYFIFO_BUFSIZ){
	cv_wait(&fifo->wr, &fifo->fifolock);
  }
  fifo->buf[fifo->nextwrite++] = d;
  fifo->nextwrite %= MYFIFO_BUFSIZ;
  fifo->state++;
  //SIGNALING
  cv_signal(&fifo->rd);
  //UNLOCKING
  spin_unlock(&fifo->fifolock);
}

unsigned long fifo_rd(struct fifo *fifo){
  unsigned long fiforead;
  spin_lock(&fifo->fifolock);
  while(fifo->state <= 0){
	printf("Reader Stream %d complete\n", ++z);
	cv_wait(&fifo->rd, &fifo->fifolock);
  }
  fiforead = fifo->buf[fifo->nextread++];
  fifo->nextread %= MYFIFO_BUFSIZ;
  fifo->state--;
  //SIGNALING
  cv_signal(&fifo->wr);
  //UNLOCKING
  spin_unlock(&fifo->fifolock);
  return(fiforead);
}
