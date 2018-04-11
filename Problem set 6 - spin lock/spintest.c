#include "spinlock.h"

int main() {
	int nchild, niter, procnum;
	int status = 0;
	pid_t f, fpast;
	int* map;
	struct spinlock *l;

	printf("Number of childs: ");
	scanf("%i", &nchild);
	if(nchild < 8) nchild = 8;
	printf("Number of iterations: ");
	scanf("%i", &niter);

	if((map = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
		fprintf(stderr, "Failed to map a int variable on virtual memory : %s\n", strerror(errno));
		return 1;
	}

	map[0] = 0;
	l = (struct spinlock *) (map + sizeof(struct spinlock));
	l->lock = map[1];

	fpast = 1;
	printf("Intial value : %d\n", *map);

	for(int i = 0; i < nchild || procnum == NPROC; i++, procnum++) {
		if(fpast > 0){ //to guarantee that only parent process forks. If the child process forks as well, twice the number of wanted children will be spawned.
			if((f = fork()) < 0) {
				fprintf(stderr, "Failed to fork from parent : %s\n", strerror(errno));
				return 1;
			}	
		}
		fpast = f;
	}
	
	if(f > 0) {
		while(wait(&status) > 0);
		printf("Proc %i, Final value : %d\n", f, *map);
	} else {
		for(int i = 0; i < niter; i++) {
			spin_lock(l);
			map[0]++;
			spin_unlock(l);
		}
		printf("Pid: %i, Ppid %i - Spin locked %i times, unlocked %i times\n", l->pid, l->ppid, l->numlock, l->numunlock);
	}
}
