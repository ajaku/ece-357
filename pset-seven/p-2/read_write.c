#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define FILE_SIZE 4096

char buf[FILE_SIZE];

int main() {
    int j;
    if ((j = read(0, buf, FILE_SIZE)) < 0) {
        perror("Read failed");
        _exit(1);
    }
    int k;
    if ((k = write(1, buf, j)) < 0) {
        perror("Write failed");
        _exit(1);
    }
    _exit(0);
}