#include "cv.h"
#define MYFIFO_BUFSIZ 1024

struct fifo{
	unsigned long buf[MYFIFO_BUFSIZ];
	struct cv full, empty;
	struct spinlock mutex;
	int read, write, count;
};

void fifo_init(struct fifo *f);
void fifo_wr(struct fifo *f,unsigned long d);
unsigned long fifo_rd(struct fifo *f);