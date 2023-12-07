#define _POSIX_SOURCE
// Preprocessor directive to include signal features

#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "spinlock.h"
#include "fifo.h"

void fifo_init(struct fifo *f) {
	memset(f, 0, sizeof(*f));
}

void fifo_wr(struct fifo *f, ulong d) {
    sigset_t oldmask, newmask;
    sigemptyset(&oldmask);
    sigemptyset(&newmask);
    int first = 1;
    	
    /* Wait until FIFO is not full */
    spin_lock(&f->my_spin);
    while (f->buf_idx >= MYFIFO_BUFSIZE) {
        // We want to add SIGUSR1 to be ignored (cond. to write)
	// because we can't write at the moment
        sigaddset(&newmask, SIGUSR1);
        
	// Block SIGUSR1
        sigprocmask(SIG_BLOCK, &newmask, &oldmask);

        // Add to pid to waitlist (only want to add once)
	if (first == 1) {
		f->write_waitlist_idx++;
		f->write_waitlist_pid[f->write_waitlist_idx] = getpid();
		f->write_waitlist_status[f->write_waitlist_idx] = 1; 
	}
	first = 0;

        // Now unlock to avoid getting stuck
        spin_unlock(&f->my_spin);
       	
	// Return to previous bitmask
	sigsuspend(&oldmask);

        // Restore mask (sigsuspend does it temporarily)
        sigprocmask(SIG_SETMASK, &oldmask, NULL);

	// Lock again
        spin_lock(&f->my_spin);
    } 
    /* FIFO no longer full */

    f->buf[f->next_write++] = d;
    f->next_write %= MYFIFO_BUFSIZE;
    f->buf_idx++;

    /* Wakeup a reader and remove from waitlist */
    for (int i = 0; i < f->read_waitlist_idx; i++) {
	if (f->read_waitlist_status[i] == 1) {
		f->read_waitlist_status[i] = 0;
		kill(f->read_waitlist_pid[i], SIGUSR1);
		break;
	}
    }
    /* Done sending signal */
    spin_unlock(&f->my_spin);
}

ulong fifo_rd(struct fifo *f) {
    ulong d;

    sigset_t oldmask, newmask;
    sigemptyset(&oldmask);
    sigemptyset(&newmask);

    spin_lock(&f->my_spin);
    int first = 1; 
    while (f->buf_idx <= 0) {
        // We want to add SIGUSR1 to be ignored (cond. to write)
	// because we can't write at the moment
        sigaddset(&newmask, SIGUSR1);
        
	// Block SIGUSR1
        sigprocmask(SIG_BLOCK, &newmask, &oldmask);

        // Add to pid to waitlist (only want to add once)
	if (first == 1) {
		f->read_waitlist_idx++;
		f->read_waitlist_pid[f->read_waitlist_idx] = getpid();
		f->read_waitlist_status[f->read_waitlist_idx] = 1; 
	}
	first = 0;

        // Now unlock to avoid getting stuck
        spin_unlock(&f->my_spin);
       	
	// Return to previous bitmask
	sigsuspend(&oldmask);

        // Restore mask (sigsuspend does it temporarily)
        sigprocmask(SIG_SETMASK, &oldmask, NULL);

	// Lock again
        spin_lock(&f->my_spin);
    }

    // Write into buffer
    d = f->buf[f->next_read++];
    f->next_read %= MYFIFO_BUFSIZE;
    f->buf_idx--;

    /* Wakeup a writer and remove from waitlist */
    for (int i = 0; i < f->write_waitlist_idx; i++) {
	if (f->write_waitlist_status[i] == 1) {
		f->write_waitlist_status[i] = 0;
		kill(f->write_waitlist_pid[i], SIGUSR1);
		break;
	}
    }
    /* Done sending signal */

    spin_unlock(&f->my_spin);
    return d;
}
