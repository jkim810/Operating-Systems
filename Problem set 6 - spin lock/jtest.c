#include "spinlock.h"

int i,k;

int main(){
  long long unsigned int procnum = 8;
  long long unsigned int iternum = 100000;
  printf("procnum = %llu\niternum = %llu\n", procnum, iternum);
  int *map = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0 );
  if(map< 0){
  	fprintf(stderr, "CRITICAL ERROR: Unable to MMAP ANON page: %s\n", strerror(errno));
  	exit(1);
  }
  map[0] = 0;
  struct spinlock *lock;
  lock = (struct spinlock *)(map+sizeof(struct spinlock));
  lock->lock= map[1];
  pid_t pid[procnum];
  for (i = 0; i < procnum; i++){
	//fork
	if((pid[i] = fork()) < 0){
  	//fork ERROR
    	fprintf(stderr, "NON-CRITICAL ERROR: Unable to fork id#%d: %s\n",i, strerror(errno));
    	return(1);
	}
	if(pid[i] == 0){
    	//spin_lock(lock);
  	 for(k = 0; k < iternum; k++){
    	map[0]++;
  	}
  	//spin_unlock(lock);
  	exit(0);
	}
  }
  for(i = 0; i < procnum; i++){
	wait(0);
  }
  printf("%d\n", map[0]);
  return(0);
}
