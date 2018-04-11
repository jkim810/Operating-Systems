#include "cv.h"

void sighandler(int signo){;}

void cv_init(struct cv *cv){
  int *map = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS| MAP_SHARED, 0, 0);
  int i;
  if(map< 0){
  	fprintf(stderr, "CRITICAL ERROR: Unable to MMAP ANONYMOUS page: %s\n", strerror(errno));
  	exit(1);
  }
  spinlock *lock;
  lock = (spinlock *)(map+sizeof(spinlock));
  cv->lock=*lock;
	for(i = 0; i < CV_MAXPROC; i++){
  	cv->pid[i] = 0;
	}
	cv->count = 0;
	signal(SIGUSR1, sighandler);
	sigfillset(&cv->sigmask);
	sigdelset(&cv->sigmask, SIGUSR1);
	//SIGNAL STUFF
}

void cv_wait(struct cv *cv, struct spinlock *mutex){
  if(cv->count >= CV_MAXPROC){
  	fprintf(stderr, "CRITICAL ERROR: Too many processes running\n");
  	exit(1);
  }
  spin_lock(&cv->lock);
  cv->pid[cv->count] = getpid();
  cv->count++;
  spin_unlock(&cv->lock);
  spin_unlock(mutex);

  //SIGNALS
  sigprocmask(SIG_BLOCK, &cv->sigmask, NULL);
  sigsuspend(&cv->sigmask);
  if(cv->count > 0){
	spin_lock(&cv->lock);
	cv->pid[cv->count - 1] = 0;
	cv->count--;
	spin_unlock(&cv->lock);
	spin_lock(mutex);
	return;
  }
  sigprocmask(SIG_UNBLOCK, &cv->sigmask, NULL);
  spin_lock(mutex);
}

int cv_broadcast(struct cv *cv){
  int wkcount = 0;
  int i;
  spin_lock(&cv->lock);
  if(cv->count == 0){
	spin_unlock(&cv->lock);
	return(0);
  }
  for(i = 0; i < CV_MAXPROC; i++){
	if(cv->pid[i] > 0){
  	kill(cv->pid[i], SIGUSR1);
  	wkcount++;
	}
  }
  spin_unlock(&cv->lock);
  return(wkcount);
}

int cv_signal(struct cv *cv){
  int wkcount = 0;
  spin_lock(&cv->lock);
  if(cv->count == 0){
	spin_unlock(&cv->lock);
	return(0);
  }
  kill(cv->pid[cv->count - 1], SIGUSR1);
  wkcount++;
  spin_unlock(&cv->lock);
  return(wkcount);
}
