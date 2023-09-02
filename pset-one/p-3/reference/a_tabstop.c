#include <stdio.h>
#include <unistd.h>
#include "my_stdio.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

typedef enum {
    IN_OUT,
    OUT,
    IN,
    NO_ARG
} state_t;

state_t parse_input(int arg, char *argv[]) {
    if (argv[1] == NULL) { return NO_ARG; }

    if (!arg) { return IN; }

    if (argv[3] == NULL) { return OUT; }

    return IN_OUT;
}

int copy_paste(struct MYSTREAM *fp_read, struct MYSTREAM *fp_write) {
    int first = 1;
    char c = myfgetc(fp_read);
    while (c != EOF) {
        if (first) { fp_write->total_bytes = fp_read->total_bytes; }
        first = 0;
        if (c == '\t') {
            fp_write->total_bytes += 3;
            for (int i = 0; i < 4; i++) {
                myfputc(' ', fp_write);
            }
        }
        else {
            myfputc(c, fp_write);
        }
        //printf("%c", c);
        c = myfgetc(fp_read);
    }

    int rv1 = myfclose(fp_read);
    int rv2 = myfclose(fp_write);

    printf("rv1: %d, rv2: %d\n", rv1, rv2);

    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    int arg = 0;
    while ((opt = getopt(argc, argv, "o")) != -1) {
        switch (opt) {
            case 'o':
                arg = 1;
                break;
        }
    }

    state_t op = parse_input(arg, argv);

    struct MYSTREAM *fp_read;
    struct MYSTREAM *fp_write;


    switch (op) {
        case IN_OUT: // Input and output files
            //char *inp = argv[3];
            //char *out = argv[2];

            fp_read = myfopen(argv[3], O_RDONLY, 4096);

            if (fp_read == NULL) {
                printf("Couldn't open file for reading\n");
                return 1;
            }

            fp_write = myfopen(argv[2], O_WRONLY, 4096);

            if (fp_write == NULL) {
                printf("Couldn't open file for writing\n");
                return 1;
            }

            return copy_paste(fp_read, fp_write);

        case OUT: // Standard input, Output fie
            fp_read = myfdopen(0, O_RDONLY, 4096);

            if (fp_read == NULL) {
                printf("Couldn't open file for reading\n");
                return 1;
            }

            fp_write = myfopen(argv[2], O_WRONLY, 4096);

            if (fp_write == NULL) {
                printf("Couldn't open file for writing\n");
                return 1;
            }

            return copy_paste(fp_read, fp_write);

        case IN: // Input file, standard out
            fp_read = myfopen(argv[1], O_RDONLY, 4096);

            if (fp_read == NULL) {
                printf("Couldn't open file for reading\n");
                return 1;
            }

            fp_write = myfdopen(1, O_WRONLY, 4096);

            if (fp_write == NULL) {
                printf("Couldn't open file for writing\n");
                return 1;
            }

            return copy_paste(fp_read, fp_write);

        case NO_ARG: // Standard input and output
            fp_read = myfdopen(0, O_RDONLY, 4096);

            if (fp_read == NULL) {
                printf("Couldn't open file for reading\n");
                return 1;
            }

            fp_write = myfdopen(1, O_WRONLY, 4096);

            if (fp_write == NULL) {
                printf("Couldn't open file for writing\n");
                return 1;
            }

            return copy_paste(fp_read, fp_write);
    }

    return 0;
}