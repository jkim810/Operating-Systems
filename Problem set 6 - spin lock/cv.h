#include "spinlock.h"
#define CV_MAXPROC 64

struct cv{
	pid_t procs[CV_MAXPROC];
	int waiters;
    sigset_t mask;
	struct spinlock mutex;
};

void cv_init(struct cv *cv);
void cv_wait(struct cv *cv, struct spinlock *mutex);
int cv_broadcast(struct cv *cv);
int cv_signal(struct cv *cv);