#include "spinlock.h"

void spin_lock(struct spinlock *l){
	while(tas(&(l->primitive_lock)) != 0){
  	;
	}
}

void spin_unlock(struct spinlock *l){
	l->primitive_lock=0;
}
