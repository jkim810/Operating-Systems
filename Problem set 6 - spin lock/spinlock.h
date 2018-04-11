#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NPROC 64

struct spinlock{
	volatile char lock;
	pid_t pid;
	pid_t ppid;
	int numlock;
	int numunlock;
};

int tas(volatile char *lock);
void spin_lock(struct spinlock *l);
void spin_unlock(struct spinlock *l);