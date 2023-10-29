#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <errno.h>

char* gen_word(char *rand_word, int nc) {

    for (int j = 0; j < nc; j++) {

        int rand_char = (rand() % 25) + 65; // Generate ASCII value from 65-89
        rand_word[j] = (char)rand_char;

    }

    return rand_word;
}

int main(int argc, char* argv[]) {

    long gen_lim; // Number of words to generate
    errno = 0;

    if (argc > 2)  { perror("Too many input arguments"); return 1; }
    if (argc == 2) { gen_lim = strtol(argv[1], NULL, 10); }
    if (errno < 0) { perror("Error converting from string to long"); return 1; }

    int nc = 4;

    if (!gen_lim) {
        for (;;) {
            char rand_word[nc];
            printf("%s\n", gen_word(rand_word, nc));
        }
    }

    for (int i = 0; i < gen_lim; i++) {
        char rand_word[nc];
        printf("%s\n", gen_word(rand_word, nc));
    }

    return 0;
}