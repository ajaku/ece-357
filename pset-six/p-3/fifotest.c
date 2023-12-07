#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "fifo.h"
#include "spinlock.h"
#include "tas.h"

// will spawn 8 child processes with shared memory regions
// each process will iterate n times (command line arg)
int main(int argc, char *argv[]) {
	if (argc != 3) { 
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

	if (n_proc > 64) {
		fprintf(stderr, "Limited to only 64 processes\n");
		return 1;
	}

	struct fifo *my_fifo = mmap(NULL, sizeof(*my_fifo),
	    PROT_READ | PROT_WRITE,
	    MAP_SHARED | MAP_ANONYMOUS,
	    -1, 0);

	/* Writer forks */
	int writer_pid[n_proc];
	for (int i = 0; i < n_proc; i++) {
		writer_pid[i] = fork();
		if (writer_pid[i] == -1) { perror("Fork failed"); return 1; }
		if (writer_pid[i] == 0) {
			for (int j = 0; j < n_itrs; j++) {
				fifo_wr(my_fifo, j);
			}
			exit(0);
		} 
	}

	/* Reader forks */
	pid_t read_pid = fork();
	if (read_pid == 0) {
		for (int i = 0; i < n_proc*n_itrs; i++) {
			printf("%ld\n", fifo_rd(my_fifo));
		}
		exit(0);
	}

	int status;
	pid_t wpid;
	while ((wpid = wait(&status)) > 0)
			;
	printf("Done with reading and writing\n");
	return 0;
}
