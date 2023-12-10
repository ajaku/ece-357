#ifndef __FIFO_H
#define __FIFO_H

#include <sys/types.h>
#include "spinlock.h"

#define MYFIFO_BUFSIZE 2
#define NPROC          2

typedef unsigned long ulong;


struct fifo {
    /* Elements of the fifo */
    int buf_idx;          // count of buffer
    int next_write, next_read;
    ulong buf[MYFIFO_BUFSIZE];
    
    /* Create a waitstack */
    
    /* Elements within waitlist */ 
    int write_waitlist_idx;     // count of waitlist
    pid_t write_waitlist_pid[NPROC]; 

    int read_waitlist_idx;     // count of waitlist
    pid_t read_waitlist_pid[NPROC]; 

    /* Mutex */
    struct spinlock my_spin;
};

void fifo_init(struct fifo *f);
/* Initialize the shared memory FIFO *f including any required underlying
* initializations (such as calling cv_init). The FIFO will have a static
* fifo length of MYFIFO_BUFSIZ elements. #define this in fifo.h.
*Avalue of 1K is reasonable. In most cases, simply setting the
* entire struct fifo to 0 will suffice as initialization.
*/

void fifo_wr(struct fifo *f, ulong d);
/* Enqueue the data word d into the FIFO, blocking unless and until the
* FIFO has room to accept it. (i.e. block until !full)
* Wake up a reader which was waiting for the FIFO to be non-empty
*/

ulong fifo_rd(struct fifo *f);
/* Dequeue the next data word from the FIFO and return it. Block unless
* and until there are available words. (i.e. block until !empty)
* Wake up a writer which was waiting for the FIFO to be non-full
*/

void empty_handler(int sig);


#endif /* __FIFO_H */
