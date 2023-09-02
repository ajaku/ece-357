#ifndef MY_STDIO_H
#define MY_STDIO_H

#include <stddef.h>

struct MYSTREAM {
    unsigned char* buf;
    int fd;
    int bufsize;
    int mode;
    size_t current_byte;
    size_t total_bytes;
};

struct MYSTREAM *myfopen(const char *pathname, int mode, int bufsize);
struct MYSTREAM *myfdopen(int filedesc, int mode, int bufsize);
int myfgetc (struct MYSTREAM *stream);
int myfputc(int c, struct MYSTREAM *stream);
int myfclose(struct MYSTREAM *stream);


#endif /*MY_STDIO_H*/