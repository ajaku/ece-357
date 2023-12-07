#ifndef __SPINLOCK_H
#define __SPINLOCK_H

struct spinlock {
	int  pid;
	char lock;
};

void spin_lock(struct spinlock *l);
void spin_unlock(struct spinlock *l);

#endif /* __SPINLOCK_H */