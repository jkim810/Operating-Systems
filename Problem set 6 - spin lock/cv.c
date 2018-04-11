#include "cv.h"

void sighandler(int signo){
	return;
}

void cv_init(struct cv *cv){
	int *map;
	struct spinlock *l;

    if((map = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0)) == MAP_FAILED) {
		fprintf(stderr, "ERRNO %d - Failed to map a int variable on virtual memory : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	l = (struct spinlock *) (map + sizeof(struct spinlock));
	cv->mutex = *l;

	for(int i = 0; i < CV_MAXPROC; i++)
		cv->procs[i] = 0;

	cv->waiters = 0;
	if(signal(SIGUSR1, sighandler) == SIG_ERR){
		fprintf(stderr, "Failed to assign signal handler for SIGUSR1: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	sigfillset(&cv->mask);
	sigdelset(&cv->mask, SIGUSR1);
}

void cv_wait(struct cv *cv, struct spinlock *m){
	spin_lock(&cv->mutex);
	cv->procs[cv->waiters++] = getpid();	
	spin_unlock(&cv->mutex);
	
	spin_unlock(m);
	if (sigprocmask(SIG_BLOCK, &(cv->mask), NULL) < 0) {
   		fprintf(stderr, "Sigprocmask from cv_wait failed: %s", strerror(errno));
   		exit(EXIT_FAILURE);
    }
    sigsuspend(&cv->mask);
    if(cv->waiters > 0){
    	spin_lock(&cv->mutex);
    	cv->procs[cv->waiters-1] = 0;
    	cv->waiters--;
    	spin_unlock(&cv->mutex);
    	spin_lock(m);
    	return;
    }

    if (sigprocmask(SIG_UNBLOCK, &(cv->mask), NULL) < 0) {
   		fprintf(stderr, "Sigprocmask from cv_wait failed: %s", strerror(errno));
   		exit(EXIT_FAILURE);
    }
    spin_lock(m);
}

int cv_broadcast(struct cv *cv){
	if(cv->waiters == 0) return 0;
	int awaken = 0;
	spin_lock(&cv->mutex);
	for(int i = 0; i < CV_MAXPROC;i++){
		if(cv->procs[i] > 0){
			kill(cv->procs[i], SIGUSR1);
			awaken++;
		}
	}
	spin_unlock(&cv->mutex);
	return awaken;
}

int cv_signal(struct cv *cv){
	if(cv->waiters == 0) return 0;
	spin_lock(&cv->mutex);
	kill(cv->procs[cv->waiters-1], SIGUSR1);
	spin_unlock(&cv->mutex);
	return 1;
}