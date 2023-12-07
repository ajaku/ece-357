#define _POSIX_SOURCE
// Preprocessor directive to include signal features

#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include "spinlock.h"
#include "fifo.h"

void fifo_init(struct fifo *f) {
    // Initialize fifo struct
    *f = {0};
}

void fifo_wr(struct fifo *f, ulong d) {
    sigset_t oldmask, newmask;
    sigemptyset(&oldmask);
    sigemptyset(&newmask);

    spin_lock(&f->my_spin);
    while (f->buf_idx >= MYFIFO_BUFSIZE) {
        // We want to add SIGUSR1 to be ignored
        sigaddset(&newmask, SIGUSR1);
        // Block those signals
        sigprocmask(SIG_BLOCK, &newmask, &oldmask);

        // Add to waitlist
        // How should I go about incrementing this? it should
        // realistically only happen once per writer?
        // So then clearly not within this while loop
        f->write_waitlist_idx++;
        f->write_waitlist[f->write_waitlist_idx] = getpid();

        // Now unlock to avoid getting stuck
        spin_unlock(&f->my_spin);
        sigsuspend(&oldmask);

        // Restore mask (sigsuspend does it temporarily)
        sigprocmask(SIG_SETMASK, &oldmask, NULL);
        spin_lock(&f->my_spin);
    } 
    f->buf[f->next_write++] = d;
    /* Enforces circularity */
    f->next_write %= MYFIFO_BUFSIZE;

    for (int i = 0; i < f->write_waitlist_idx; i++) {
        // Send SIGUSR1 to each pending writer
        kill(SIGUSR1, f->write_waitlist[i]);
    }
    spin_unlock(&f->my_spin);
}

/*ulong fifo_rd(struct fifo *f) {
    ulong d;
    spin_lock(&f->my_spin);

    while (f->c_buf <= 0) {
        // wait condition
    }
    d = f->buf[f->next_read++];
    //Enforces circularity
    f->next_read %= MYFIFO_BUFSIZE;
    f->c_buf--;

    // send signal condition

    spin_unlock(&f->my_spin);
    return d;
}*/