
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
	if (argc != 4) { 
		fprintf(stderr, "Incorrect number of arguments\n");
		return 1;
	}
	
	int n_proc;
	int n_itrs;
	// using strtol instead of atoi (checks for errors)
	if (!(n_proc = atoi(argv[1]))) {
		fprintf(stderr, "Failed to convert to int\n");
		return 1;
	}

	if (!(n_itrs = atoi(argv[2]))) {
		fprintf(stderr, "Failed to convert to int\n");
		return 1;
	}

	if ((n_proc < 0) || (n_itrs < 0)) {
		fprintf(stderr, "Arguments must be greater than or equal to zero\n");
		return 1;
	}

	int *shared_int = mmap(NULL, 4096,
	    PROT_READ | PROT_WRITE,
	    MAP_SHARED | MAP_ANONYMOUS,
	    -1, 0);

	*shared_int  = 0;

	int lock_flag;
	struct spinlock *my_spin;
	if (argv[3]) {
		if (strcmp(argv[3], "lock") != 0 && strcmp(argv[3], "unlock") != 0) {
			fprintf(stderr, "Argument must lock or unlock\n");
			return 1;
		}

		if ((lock_flag = strcmp(argv[3], "unlock")) != 0) {
			lock_flag = 1;
		}

		my_spin = mmap(NULL, sizeof(*my_spin),
				PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS,
				-1, 0);
	}

	int pid[n_proc];
	for (int i = 0; i < n_proc; i++) {
		pid[i] = fork();
		if (pid[i] == -1) { perror("Fork failed"); return 1; }
		if (pid[i] == 0) {
			for (int j = 0; j < n_itrs; j++) {
				if (lock_flag) {
					spin_lock(my_spin);
					*shared_int += 1;
					spin_unlock(my_spin);
				} else {
					*shared_int += 1;
				}
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
