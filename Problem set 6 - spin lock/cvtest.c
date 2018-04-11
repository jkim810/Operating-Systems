#include "cv.h"

int pid, i;

int main(){
	struct spinlock testlock;
	struct cv *cv;
	cv = (struct cv *) mmap(NULL, sizeof(cv), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	cv_init(cv);
	for(i = 0; i < 3; i++){
		if((pid = fork()) < 0){
			//fork ERROR
			fprintf(stderr, "NON-CRITICAL ERROR: Unable to fork id#%d: %s\n",i, strerror(errno));
			return 1;
		}
		if(pid == 0){
			printf("Fork Succeeded, Wait\n");
			cv_wait(cv, &testlock);
			exit(0);
		}
	}
	sleep(2);
	printf("Waking up everything\n");
	int wakeup = cv_broadcast(cv);
	for(i = 0; i < 2; i++){
		printf("Waiting for children to exit\n");
		wait(0);
	}
	printf("Wakeup Num = %d\n", wakeup);
	return 0;
}