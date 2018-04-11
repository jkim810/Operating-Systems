#include "spinlock.h"

void spin_lock(struct spinlock *l){
	while(tas(&l->lock)!=0) sched_yield();
	l->numlock++;
	l->pid = getpid();
	l->ppid = getppid();
}
void spin_unlock(struct spinlock *l){
	l->lock = 0;
	l->numunlock++;
}
