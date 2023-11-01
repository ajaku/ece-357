#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    errno = 0;

    if (argc > 2)  { fprintf(stderr, "Too many input arguments\n"); return 1; }
    if (argc < 2)  { fprintf(stderr, "Too few input arguments\n"); return 1; }

    FILE *in_file = fopen(argv[1], "r");
    if (in_file == NULL) {
        perror("Failed to open dictionary");        
        return 1;
    }


    int dict_capacity = 100000;
    int dict_size = 0;
    char **dict;

    if ( (dict = (char **)malloc(dict_capacity*sizeof(char *))) == NULL) {
        perror("Initial dictionary malloc failed");
        return 1;
    }; // pre allocate for an array of size 100,000
    

    ssize_t nread;
    size_t len = 0;
    char *line = NULL;

    // It is implicitly assumed that no input would be larger than 4096 characters
    while ((nread = getline(&line, &len, in_file)) != -1) {

        if (dict_size == dict_capacity - 1) {
            // reallocate
            dict_capacity = dict_size*2;
            if ((dict = (char **)realloc(dict, dict_capacity*sizeof(char *))) == NULL) {
                perror("Realloc for dictionary failed");
                return 1;
            }
        }

        *(dict + dict_size) = strdup(line); 
        dict_size++;
    }

    // Done placing items into dict, now check stdin for matches
    nread = 0;
    len = 0;
    line = NULL;

    // It is implicitly assumed that no input would be larger than 4096 characters
    while ((nread = getline(&line, &len, stdin)) != -1) {
        for (int i = 0; i < dict_size; i++) {
            // Loop through dictionary string and make upper
            // Credit: https://stackoverflow.com/questions/35181913/converting-char-to-uppercase-in-c 
            char *s = dict[i];
            while (*s) {
                *s = toupper((unsigned char) *s);
                s++;
            }
            if (!strcmp(line, dict[i])) {
                fprintf(stdout, "%s", line);
            }
        }
    }

    return 0;
}
