#include <stdio.h>
#include <unistd.h>
#include "my_stdio.h"

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

int main(int argc, char *argv[])
{
    int opt;
    int arg;
    while ((opt = getopt(argc, argv, "of")) != -1) {
        switch (opt) {
            case 'o':
                arg = 1;
                break;
        }
    }

    state_t op = parse_input(arg, argv);

    switch (op) {
        case IN_OUT: // Input and output files
            char *inp = argv[3];
            char *out = argv[2];

            FILE *fp_read;
            char c;

            fp_read = fopen(inp, "r");

            if (fp_read == NULL) {
                printf("Couldn't open file\n");
                return 1;
            }

            FILE *fp_write;

            fp_write = fopen(out, "w");

            if (fp_write == NULL) {
                printf("Couldn't open write file\n");
                return 1;
            }

            c = fgetc(fp_read);
            while (c != EOF) {
                printf("%c\n", c);
                if (c == '\t') {
                    printf("TAB!\n");
                    for (int i = 0; i < 4; i++) {
                        fputc(' ', fp_write);
                    }
                } else {
                    fputc(c, fp_write);
                }
                c = fgetc(fp_read);
            }

            printf("Closing the files\n");
            fclose(fp_read);
            fclose(fp_write);

            return 0;

        case OUT: // Standard input, Output fie

            break;

        case IN: // Input file, standard out

            break;

        case NO_ARG: // Standard input and output

            break;
    }

    return 0;
}