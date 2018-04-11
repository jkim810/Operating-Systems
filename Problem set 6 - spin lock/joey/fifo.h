#include "cv.h"

#define MYFIFO_BUFSIZ 1024

typedef struct fifo{
	unsigned long buf[MYFIFO_BUFSIZ];
	int state;
	int nextread;
	int nextwrite;
	spinlock fifolock;
	cv wr;
	cv rd;
}fifo;

void fifo_init(struct fifo *f);
/* Initialize the shared memory FIFO *f including any required underlying
 * initializations (such as calling cv_init).  The FIFO will have a static
 * fifo length of MYFIFO_BUFSIZ elements.   #define this in fifo.h.
 * A value of 1K is reasonable.
*/
void fifo_wr(struct fifo *f,unsigned long d);
/* Enqueue the data word d into the FIFO, blocking unless and until the
 * FIFO has room to accept it. (i.e. block until !full)
 */
unsigned long fifo_rd(struct fifo *f);
/* Dequeue the next data word from the FIFO and return it.  Block unless
 * and until there are available words.  (i.e. block until !empty)
 */
