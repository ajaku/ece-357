
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
	
	int n_proc;
	int n_itrs;
	// using strtol instead of atoi (checks for errors)
	if (!(n_proc = atoi(argv[1]))) {
		perror("Failed to convert to int");
		return 1;
	}

	if (!(n_itrs = atoi(argv[2]))) {
		perror("Failed to convert to int");
		return 1;
	}

	int *shared_int = mmap(NULL, 4096,
	    PROT_READ | PROT_WRITE,
	    MAP_SHARED | MAP_ANONYMOUS,
	    -1, 0);

	*shared_int  = 0;

	int pid[n_proc];
	for (int i = 0; i < n_proc; i++) {
		pid[i] = fork();
		if (pid[i] == -1) { perror("Fork failed"); return 1; }
		if (pid[i] == 0) {
			for (int j = 0; j < n_itrs; j++) {
				struct spinlock *my_spin;
				//my_spin = (struct spinlock *)malloc(sizeof(struct spinlock));
				my_spin = mmap(NULL, sizeof(*my_spin),
						PROT_READ | PROT_WRITE,
						MAP_SHARED | MAP_ANONYMOUS,
						-1, 0);
				spin_lock(my_spin);
				*shared_int += 1;
				spin_unlock(my_spin);
			}
			exit(0);
		} 
	}

	int status;
	pid_t wpid;
	while ((wpid = wait(&status)) > 0)
			;
	printf("From parent: %d\n", *shared_int);
	return 0;
}
