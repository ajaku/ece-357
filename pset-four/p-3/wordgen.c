#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <errno.h>

char* gen_word(char *rand_word, int word_size) {
    for (int j = 0; j < word_size; j++) {

        int rand_char = (rand() % 26) + 65; // Generate ASCII value from 65-89
        rand_word[j] = (char)rand_char;

    }

    return rand_word;
}

int main(int argc, char* argv[]) {

    long gen_lim; // Number of words to generate
    errno = 0;

    srand(time(NULL)); // Generate new random seed each time

    if (argc > 2)  { perror("Too many input arguments"); return 1; }
    if (argc == 2) { gen_lim = strtol(argv[1], NULL, 10); }
    if (errno < 0) { perror("Error converting from string to long"); return 1; }

    int nc = 6;
    int word_size;

    if (!gen_lim) {

        for (;;) {

            if ((word_size = (rand()%(nc - 3 + 1)) + 3) < 3) {
                fprintf(stderr, "Random generation failed\n");
            }

            char *rand_word;
            if ((rand_word = (char *)calloc(word_size, sizeof(char *))) == NULL) {
                fprintf(stderr, "Memory allocation for words failed");
            }
            printf("%s\n", gen_word(rand_word, nc));
            free(rand_word);
        }
    }

    for (int i = 0; i < gen_lim; i++) {

        if ((word_size = (rand()%(nc - 3 + 1)) + 3) < 3) {
            fprintf(stderr, "Random generation failed\n");
        }
        
        char *rand_word;
        if ((rand_word = (char *)calloc(word_size, sizeof(char *))) == NULL) {
            fprintf(stderr, "Memory allocation for words failed");
        }
        printf("%s\n", gen_word(rand_word, word_size));
        free(rand_word);
    }

    return 0;
}
