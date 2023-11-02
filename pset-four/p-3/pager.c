#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    errno = 0;

    if (argc > 1)  { fprintf(stderr, "Too many input arguments\n"); return 1; }

    FILE *keyboard_inp;

    if ((keyboard_inp = fopen("/dev/tty", "r")) == NULL) {
        perror("Opening /dev/tty failed");
        return 1;
    }

    ssize_t nread;
    size_t len = 0;
    char *line = NULL;

    // It is assumed that no input would be larger than 4096 characters
    int i = 0;
    while ((nread = getline(&line, &len, stdin)) != -1) {
        fprintf(stdout, "%s", line);
        i++;   
        if (i == 23) {
            fprintf(stdout, "---Press RETURN for more---");
            int key;
            while ((key = fgetc(keyboard_inp)) != EOF) {
                if (key == 'q' || key == 'Q') {
                    fprintf(stdout, "*** Pager terminated by Q command ***\n");
                    return 0;
                }
                if (key == '\n') {
                    i = 0;
                    break;
                }
            }
        }
    }
    return 0;
}
