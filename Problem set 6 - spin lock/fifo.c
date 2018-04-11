#include "fifo.h"

int i;

void fifo_init(struct fifo *f) {
	struct cv *rd = NULL;
	struct cv *wr = NULL;
	
	if((rd = (struct cv *) mmap(NULL, sizeof(struct cv), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
   		fprintf(stderr, "Failed to mmap read cv: %s\n", strerror(errno));
   		exit(EXIT_FAILURE);
    }

   	if((wr = (struct cv *) mmap(NULL, sizeof(struct cv), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
   		fprintf(stderr, "Failed to mmap write cv: %s\n", strerror(errno));
   		exit(EXIT_FAILURE);
    }

    f->full = *wr;
    f->empty = *rd;
	cv_init(&f->full);
	cv_init(&f->empty);
	f->read = 0;
	f->write = 0;
	f->count = 0;
	spin_unlock(&f->mutex);
}

void fifo_wr(struct fifo *f, unsigned long d){
	spin_lock(&f->mutex);
	while(f->count >= MYFIFO_BUFSIZ)
		cv_wait(&f->full, &f->mutex);
	f->buf[f->write++] = d;
	f->write%=MYFIFO_BUFSIZ;
	f->count++;
	cv_signal(&f->empty);
	spin_unlock(&f->mutex);
}

unsigned long fifo_rd(struct fifo *f){
	unsigned long d;
	spin_lock(&f->mutex);
	while(f->count <= 0){
		cv_wait(&f->empty, &f->mutex);
		printf("Reader %d completed\n", i++);
	}
	d = f->buf[f->read++];
	f->read%=MYFIFO_BUFSIZ;
	f->count--;
	cv_signal(&f->full);
	spin_unlock(&f->mutex);
	return d;
}	