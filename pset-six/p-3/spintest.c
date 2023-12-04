
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "spinlock.h"
#include "tas.h"

// will spawn 8 child processes with shared memory regions
// each process will iterate n times (command line arg)
int main(int argc, char *argv[]) {
	if (argc != 3) { 
		fprintf(stderr, "Incorrect number of arguments");
		return 1;
	}

	struct spinlock *my_spin;
//	my_spin = mmap(NULL, sizeof(*my_spin), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
	my_spin = (struct spinlock *)malloc(sizeof(struct spinlock));
	printf("Before lock: %d\n", (my_spin->lock));
	spin_lock(my_spin);
	printf("After lock: %d\n", (my_spin->lock));
	spin_unlock(my_spin);
	printf("After unlock: %d\n", (my_spin->lock));
	return 0;
}
