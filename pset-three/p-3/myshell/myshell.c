#include <stdio.h>
#include <errno.h>
#include <string.h> // strerr

#define MAX_READ 4096

int main(int argc, char* argv[]) {
    errno = 0;
    // Invalid input
    if (argc > 2) { fprintf(stderr, "Too many inputs\n"); return -1; }
    // Script input
    int read_script = 0;
    FILE *script;
    if (argc == 2) {
        printf("Shell: %s\nScript: %s\n", argv[0], argv[1]);
        if(!(script = fopen(argv[1], "r"))) {
            fprintf(stderr, "Error opening input script %s - %s\n", argv[1], strerror(errno));
            return -1;
        }
        read_script = 1;
    }

    int end_read = 0;
    while (!end_read) {
        char rb[MAX_READ] = "";
        // Have not addressed possible partial reads/commands being larger than max
        if (!read_script) {
            if (!fgets(rb, MAX_READ, stdin)) {
                return -1;
            }
        } else {
            if (!fgets(rb, MAX_READ, script)) {
                return -1;
            }
        }

        // Source: https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
        char *pos;
        if ((pos=strchr(rb, '\n')) != NULL)
        *pos = '\0';
        else
        printf("error\n");
        /* input too long for buffer, flag error */
        
        // Parse each line and populate array with items
        char *token;
        char *command; 
        char *flags[MAX_READ]; // statically preallocated
        char *arguments[MAX_READ]; // statically preallocated
        char *redirections[MAX_READ]; // statically preallocated
        int a_idx, r_idx;

        if (!(token = strtok(rb, "  "))) {
            fprintf(stderr, "Error tokenizing input %s - %s\n", rb, strerror(errno));
            return -1;
        }
        printf("Token: %s\n", token);
        command = token;
        while ((token = strtok(NULL, " ")) != NULL) {
            printf("Token: %s\n", token);
            int elements = 0;
            int i = 0;
            while (*(token+i)) {
                elements++;
                i++;
            }

            if (((*(token) == '-') && ((*token+1) == '-')) && (elements >= 2)) {
                printf("-- argument\n");
            } else if (*(token) == '-'){
                printf("- argument\n");
            } 

            if (*(token) == '<') {
                printf("< redirection\n");
            } 

            if (((*(token) == '>') && (*(token+1) == '>')) && (elements >= 2)) {
                printf(">> redirection\n");
            } else if (*(token) == '>') {
                printf("> redirection\n");
            } 

            if (((*(token) == '2') && (*(token+1) == '>') && (*(token+2) == '>')) && (elements >= 3)) {
                printf("2>> redirection\n");
            } else if (((*(token) == '2') && (*(token+1) == '>')) && (elements >= 2)) {
                printf("2> redirection\n");
            } 
        }
    }
    return 0;
}