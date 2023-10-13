#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> // strerr

#define MAX_READ 4096

// Gets size of char * passed
int get_size(char *str) {
    int elements = 0;
    int i = 0;
    while (*(str+i)) {
        elements++;
        i++;
    }
    return i;
}

int str_comp(char *str_1, char *str_2, int j) {
    int match = 1;
    for (int i = 0; i < j; i++) {
        if (!(*(str_1+ i) == *(str_2 + i))) {
            match = 0;
        }
    }
    return match;
}

// Create substring
// Remember to free afterwards 
char *my_dupe(char *str, int start) {
    if (start == 0) {
        return strdup(str);
    }
    
    if (start < 0) { return NULL; }

    int size = get_size(str);
    int my_size = size - start;
    char *my_str = (char *)malloc(my_size*sizeof(char));

    for (int i = start; i < size; i++) {
        *(my_str + i - start) = *(str + i);
    }

    return my_str;
}


int main(int argc, char* argv[]) {
    errno = 0;
    if (argc > 2) { fprintf(stderr, "Too many inputs\n"); return -1; }
    FILE *script = NULL;
    //int r_script;
    if (argc == 2) {
        printf("Shell: %s\nScript: %s\n", argv[0], argv[1]);
        if(!(script = fopen(argv[1], "r"))) {
            fprintf(stderr, "Error opening input script %s - %s\n", argv[1], strerror(errno));
            return -1;
        }
        //r_script = 1;
    }

    while(1) {
        size_t input_size = MAX_READ;
        size_t line;
        char *input = (char *)malloc(input_size * sizeof(char)); // Remember to free!
        if(input == NULL) {
            fprintf(stderr, "Unable to allocate buffer\n");
            exit(1);
        }
        line = getline(&input, &input_size, stdin);
        if (line <= 0) {
            // Skip clause
            // Define what to do if we skip
            fprintf(stderr, "Failed to read line from %s - %s\n", input, strerror(errno));
        }
        printf("Line: %s\n", input);
    
        char *token;
        if (!(token = strtok(input, " "))) {
            fprintf(stderr, "Error tokenizing input %s - %s\n", input, strerror(errno));
            return -1;
        }

        // Represent potential tokens from each input
        char *command, *o_stdin, *oct_stdout, *oct_stderr, *oca_stdout, *oca_stderr = NULL;
        char **args = NULL;
        printf("Token: %s\n", token);
        command = my_dupe(token, 0);
        while ((token = strtok(NULL, " \n")) != NULL) {
            int parsed = 0;
            printf("Token: %s\n", token);
            int elements = strlen(token);
            
            // Check for 2>>
            if ((elements >= 3) && (!parsed)) {
                char test_stderr[3] = "2>>";

                if (strncmp(test_stderr, token, 3)) { 
                    parsed = 1;
                    oca_stderr = my_dupe(token, 3);
                    printf("2>> redirection: %s\n", oca_stderr);
                }
            } 

            if ((elements >= 2) && (!parsed)) {
                char test_stdout[2] = ">>";
                char test_stderr[2] = "2>";
                char test_args[2] = "--";

                if (strncmp(test_stderr, token, 2)) { 
                    parsed = 1;
                    oct_stderr = my_dupe(token, 2);
                    printf("2> redirection: %s\n", oct_stderr);
                }

                if (strncmp(test_stdout, token, 2)) { 
                    parsed = 1;
                    oct_stdout = my_dupe(token, 2);
                    printf(">> redirection: %s\n", oct_stdout);
                }

                if (strncmp(test_args, token, 2)) {
                    parsed = 1;
                    // What do I do with args!
                    oct_stdout = my_dupe(token, 2);
                    printf(">> redirection: %s\n", oct_stdout);
                }
            } 

             if ((elements >= 1) && (!parsed)) {
                char *test_stdout = ">";
                char *test_stdin = "<";

                if (str_comp(token, test_stdin, 1)) {
                    parsed = 1;
                    o_stdin = my_dupe(token, 1);
                    printf("< redirection: %s\n", o_stdin);
                }

                if (str_comp(token, test_stdout, 1)) {
                    parsed = 1;
                    oct_stdout = my_dupe(token, 1);
                    printf("> redirection: %s\n", oct_stdout);
                }
            }
        }
        // Get the correct size of each token, now parse substrings
        printf("\n");
    }
}