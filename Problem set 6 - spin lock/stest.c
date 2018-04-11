#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

static const int SIZE = sizeof(int);

//max number of processers
#define N_PROC 64

int tas(volatile char *lock);

int main() {
    pid_t my_procnum = -1;
    int procNum = 4;//number of processors
    int status = 0;
    pid_t f;
    int* map;
    char* lock;    
    if((map = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == 
    MAP_FAILED) {
   	 fprintf(stderr, "ERRNO %d - Failed to map a int variable on virtual memory : %s\n", 
 errno, strerror(errno));
   	 return 1;
    }    
    if((lock = mmap(NULL, sizeof(char), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 
    -1, 0)) == MAP_FAILED) {
   	 fprintf(stderr, "ERRNO %d - Failed to map a char on virtual memory : %s\n", 
 errno, strerror(errno));
   	 return 1;
    }
    
    *map = 0;
    printf("Intial value : %d\n", *map);
    
    //create children equal to procNum, excluding parent
    int i = 0;
    do {
   	 if((f = fork()) < 0) {
   		 fprintf(stderr, "ERRNO %d - Failed to fork from parent : %s\n", errno, 
 strerror(errno));    
   		 return 1;
   	 }
   	 my_procnum++;
   	 i++;
    } while(i < procNum && f > 0 && my_procnum < N_PROC);
    
    if(f > 0) {    
   	 //parent just waits for all children
   	 while(wait(&status) > 0);
   	 printf("Final value : %d\n", *map);   	 
    } else {    
   	 for(int j = 0;j < 1000000;j++) {
   		 //while(tas(lock)!=0);//tas
   		 *map += 10;
   		 //*lock = 0;
   	 }   	 
    }
}
