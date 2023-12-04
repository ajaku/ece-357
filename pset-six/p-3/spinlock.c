#include <stdio.h>
#include <errno.h>
#include <sched.h>
#include "spinlock.h"
#include "tas.h"

void spin_lock(struct spinlock *l) {
	// When lock is acquired tas returns a 0
	while (tas(&l->lock) != 0) {
		if (sched_yield() < 0) {
			perror("Schedule yielding failed");
		}
	}
}

void spin_unlock(struct spinlock *l) {
	l->lock = 0;
}
