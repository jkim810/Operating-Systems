#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

typedef struct spinlock{
    volatile char primitive_lock;
}spinlock;

int tas(volatile char *lock);
void spin_lock(struct spinlock *l);
void spin_unlock(struct spinlock *l);
