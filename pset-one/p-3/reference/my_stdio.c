#include "my_stdio.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

struct MYSTREAM *myfopen(const char *pathname, int mode, int bufsize) {
    // Perform preliminary error checking
    int fileExists = access(pathname, F_OK); // check to see if file exists
    int fd;

    if (((mode != O_WRONLY) && (mode != O_RDONLY))|| bufsize < 0) {
        errno = EINVAL;
        return NULL;
    }

    if (mode == O_WRONLY) { //WRONLY mode
        // First step: Check to see if file exists
        if (fileExists) {
            fd = open(pathname, O_WRONLY | O_TRUNC);
        }
        else {
            fd = open(pathname, O_WRONLY | O_CREAT, 0777);
        }
    }

    if (mode == O_RDONLY) { // RDONLY mode
        if (fileExists) {
            fd = open(pathname, O_RDONLY);
        } else {
            fd = open(pathname, O_RDONLY, 0444);
        }
    }

    struct MYSTREAM *stream = malloc(sizeof(struct MYSTREAM));
    if (stream == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    stream->buf = malloc(sizeof(stream->buf));
    if (stream->buf == NULL) {
        free(stream);
        errno = ENOMEM;
        return NULL;
    }

    stream->fd      = fd;
    stream->bufsize = bufsize;
    stream->mode    = mode;
    return stream;
}

struct MYSTREAM *myfdopen(int filedesc, int mode, int bufsize) {
   if (bufsize < 0) {
        errno = EINVAL;
        return NULL;
   }

    struct MYSTREAM *stream = malloc(sizeof(struct MYSTREAM));
    if (stream == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    stream->buf = malloc(sizeof(stream->buf));
    if (stream->buf == NULL) {
        free(stream);
        errno = ENOMEM;
        return NULL;
    }

    stream->fd      = filedesc;
    stream->bufsize = bufsize;
    return stream;
}

// If buffer is empty, attempt to load 4096 bytes (could not load 4096)
// Go through loaded bytes and incrementaly return characters in buffer until EOF or out of memory
// If out of memroy, reset ptr to 0 and load more bytes
// Repeat until EOF or Error

int myfgetc (struct MYSTREAM *stream) {
    // Occurs only when file is first open
    if (stream->total_bytes == 0) { // Occurs when file is first open
        stream->current_byte = 0;

        size_t loaded_bytes = read(stream->fd, stream->buf, 4096);
        stream->total_bytes += loaded_bytes;

        /*
         * Up to here, we've loaded x bytes into buffer (good up to 4096 chars)
         * The buffer properly contains those values
         */
        if (loaded_bytes == 0) { // End of file
            errno = 0;
            return -1;
        }

        if (loaded_bytes == -1) { // Error
            return -1;
        }
    }

    if (stream->current_byte < stream->total_bytes) {
        // This also appears to be working as intended
        stream->current_byte += 1;
        //printf("Index: %ld\n", stream->current_byte);
        //printf("char: %c\n\n", *stream->buf);
        return *(stream->buf++);
    }

    // Occurs once buffer iteration reaches total number of bytes read and need to check if there's more in the file
    size_t loaded_bytes = read(stream->fd, stream->buf, stream->bufsize);

    stream->total_bytes += loaded_bytes; // Reload buffer

    if (loaded_bytes == 0) { // End of file
        errno = 0;
        return -1;
    }

    if (loaded_bytes == -1) { // Error
        return -1;
    }

    return *stream->buf++;
}

int myfputc(int c, struct MYSTREAM *stream) {
    //printf("Char: %c\n", c);
    *(stream->buf+(stream->current_byte)) = c;
    stream->current_byte++;

    //printf("Curr: %ld\n", stream->current_byte);
    //printf("Total: %ld\n", stream->total_bytes);
    //printf("Buffer: %s\n", stream->buf);

    if ((stream->current_byte == stream->total_bytes) || (stream->current_byte == stream->bufsize)) {
        //printf("Flush buf val = %s\n", stream->buf);
        int rv = write(stream->fd, stream->buf, stream->current_byte);
        //printf("\nFLUSH BUFFER\n");
        //perror("Error val is: ");
        if (rv == -1) {
            return -1;
        }

        if (rv == 0) {
            return 0;
        }
    }

    return c;
}

int myfclose(struct MYSTREAM *stream) {
    // Read only
    if (stream->mode == O_RDONLY) {
        int rv = close(stream->fd);
        free(stream);
        return rv;
    }

    // Write only
    int write_rv = write(stream->fd, stream->buf, stream->current_byte);
    int close_rv = close(stream->fd);

    if ((write_rv | close_rv) == -1) return -1;

    return 0;
}