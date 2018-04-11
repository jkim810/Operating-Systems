#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdint.h>

void empty() {}

int main() {
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i = 0;i < 1000000;i++)
   		//empty();
   		getuid();
   		//;
    clock_gettime(CLOCK_MONOTONIC, &end);
    unsigned long nanodiff = 100000000L *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("%lu nanoseconds elapsed.\n", nanodiff);
}
