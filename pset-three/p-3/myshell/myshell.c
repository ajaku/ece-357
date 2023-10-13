#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> // strerr
#include <unistd.h> // fork

#define MAX_READ 4096

extern char **environ;
char cwd[MAX_READ];

typedef struct parsed_s {
    char *command, *o_stdin, *oct_stdout, *oct_stderr, *oca_stdout, *oca_stderr;
    int args_idx, targets_idx;
    char *args[MAX_READ]; 
    char *targets[MAX_READ];
} parsed_t;

// Create substring
// Remember to free afterwards 
char *my_dupe(char *str, int start) {
    if (start == 0) {
        return strdup(str);
    }
    
    if (start < 0) { return NULL; }

    int size = strlen(str);
    int my_size = size - start;
    char *my_str = (char *)malloc(my_size*sizeof(char));

    for (int i = start; i < size; i++) {
        *(my_str + i - start) = *(str + i);
    }

    return my_str;
}

char *my_pwd(char *buf) {
    if (getcwd(buf, MAX_READ) == NULL) {
        fprintf(stderr, "Error performing pwd: %s\n", strerror(errno));
        return NULL;
    }
    return buf;
}

int my_cd(char *dir) {
    int result;
    if (!dir) { 
        if ((result = chdir(getenv("HOME"))) == -1) {
            fprintf(stderr, "Error changing to default directory: %s\n", strerror(errno));
            return 1;
        }
    } else if ((result = chdir(dir)) == -1){
        fprintf(stderr, "Error changing to the %s directory: %s\n", dir, strerror(errno));
        return 1;
    }
    return 0;
}

int parse_input(char *input, parsed_t *params) {
    char *token;

    if (!(token = strtok(input, " "))) {
        fprintf(stderr, "Error tokenizing input %s - %s\n", input, strerror(errno));
        return 1;
    }

    params->command = my_dupe(token, 0);
    if (!(strncmp(token, "#", 1))) {
        printf("Ignore this line\n");
        return 1;
    }

    while ((token = strtok(NULL, " \n")) != NULL) {
        int elements = strlen(token);
        int parsed = 0;

        // Check for 2>>
        if ((elements >= 3) && (!parsed)) {
            char test_stderr[3] = "2>>";
            if (!strncmp(test_stderr, token, 3)) { 
                parsed = 1;
                params->oca_stderr = my_dupe(token, 3);
                printf("2>> redirection: %s\n", params->oca_stderr);
            }
        } 

        if ((elements >= 2) && (!parsed)) {
            char test_stdout[2] = ">>";
            char test_stderr[2] = "2>";
            char test_args[2]   = "--";

            if (!strncmp(test_stderr, token, 2)) { 
                parsed = 1;
                params->oct_stderr = my_dupe(token, 2);
                printf("2> redirection: %s\n", params->oct_stderr);
            }

            if (!strncmp(test_stdout, token, 2)) { 
                parsed = 1;
                params->oca_stdout = my_dupe(token, 2);
                printf(">> redirection: %s\n", params->oca_stdout);
            }

            if (!strncmp(test_args, token, 2)) {
                parsed = 1;

                if (params->args_idx < MAX_READ) {
                    params->args[params->args_idx] = my_dupe(token, 2);
                } else { fprintf(stderr, "Too many arguments!\n"); }

                printf("-- argument at %d : %s\n", params->args_idx, params->args[params->args_idx]);
                params->args_idx++;
            }
        } 

         if ((elements >= 1) && (!parsed)) {
            char *test_stdout = ">";
            char *test_stdin  = "<";
            char *test_args   = "-";

            if (!strncmp(token, test_stdin, 1)) {
                parsed = 1;
                params->o_stdin = my_dupe(token, 1);
                printf("< redirection: %s\n", params->o_stdin);
            }

            if (!strncmp(token, test_stdout, 1)) {
                parsed = 1;
                params->oct_stdout = my_dupe(token, 1);
                printf("> redirection: %s\n", params->oct_stdout);
            }

            if (!strncmp(test_args, token, 1)) {
                parsed = 1;

                if (params->args_idx < MAX_READ) {
                    params->args[params->args_idx] = my_dupe(token, 1);
                } else { fprintf(stderr, "Too many arguments!\n"); }

                printf("- argument at %d : %s\n", params->args_idx, params->args[params->args_idx]);
                params->args_idx++;
            }

            // Would only get this far assuming we haven't checked any other box
            if ((!parsed)) {
                parsed = 1;

                if (params->targets_idx < MAX_READ) {
                    params->targets[params->targets_idx] = my_dupe(token, 0);
                } else { fprintf(stderr, "Too many target files!\n"); }

                printf("Target at %d : %s\n", params->targets_idx, params->targets[params->targets_idx]);
                params->targets_idx++;
            }
        }
    }
    return 0;
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
    
        parsed_t my_params = { .command = NULL, .o_stdin = NULL, .oct_stdout = NULL, .oct_stderr = NULL, .oca_stdout = NULL, .oca_stderr = NULL,
                               .args_idx = 0, .targets_idx = 0 };

        if (parse_input(input, &my_params)) { fprintf(stderr, "Error parsing %s, skipping.\n", input); my_params.command = NULL; }



        // Procedure: Get enviornment path
        char *env = getenv("PATH");
        // Tokenize the path
        char *path_tok = strtok(env, ":");
        while (path_tok != NULL) {
            printf("p_tok: %s\n", path_tok);
            path_tok = strtok(NULL, ":");
        }
        //printf("CWD: %s\n", my_pwd(cwd));
        //my_cd(NULL);
        //chdir("..");
        //printf("CWD: %s\n", my_pwd(cwd));
        // Once you have the path, tokenize each entry and "cd" into each one (save a copy of current dir)
        // Now look for your executable and once you've found your it, go back to 
        /*
        int pid;
        int i = 10;
        switch (pid=fork()) {
            case -1:
                fprintf(stderr, "Fork failed!\n"); exit(1);
                break;
            case 0:
                printf("In child\n");
                i = 1;
                break;
            default:
                printf("In parent, new pid is %d\n", pid);
                break;
        }
        printf("pid == %d i == %d\n", pid, i);
        */

        // Parsing is done! Now move onto running the commands based on input
        // fork -> redirection -> exec
    }
}

/*
char *token;
        if (!(token = strtok(input, " "))) {
            fprintf(stderr, "Error tokenizing input %s - %s\n", input, strerror(errno));
            return -1;
        }

        // Represent potential tokens from each input
        char *command, *o_stdin, *oct_stdout, *oct_stderr, *oca_stdout, *oca_stderr= NULL;

        int args_idx = 0;
        int targets_idx = 0;
        char *args[MAX_READ]; 
        char *targets[MAX_READ];

        command = my_dupe(token, 0);

        while ((token = strtok(NULL, " \n")) != NULL) {
            int elements = strlen(token);
            int parsed = 0;
            
            // Check for 2>>
            if ((elements >= 3) && (!parsed)) {
                char test_stderr[3] = "2>>";
                if (!strncmp(test_stderr, token, 3)) { 
                    parsed = 1;
                    oca_stderr = my_dupe(token, 3);
                    printf("2>> redirection: %s\n", oca_stderr);
                }
            } 

            if ((elements >= 2) && (!parsed)) {
                char test_stdout[2] = ">>";
                char test_stderr[2] = "2>";
                char test_args[2]   = "--";

                if (!strncmp(test_stderr, token, 2)) { 
                    parsed = 1;
                    oct_stderr = my_dupe(token, 2);
                    printf("2> redirection: %s\n", oct_stderr);
                }

                if (!strncmp(test_stdout, token, 2)) { 
                    parsed = 1;
                    oca_stdout = my_dupe(token, 2);
                    printf(">> redirection: %s\n", oca_stdout);
                }

                if (!strncmp(test_args, token, 2)) {
                    parsed = 1;

                    if (args_idx < MAX_READ) {
                        args[args_idx] = my_dupe(token, 2);
                    } else { fprintf(stderr, "Too many arguments!\n"); }

                    printf("-- argument at %d : %s\n", args_idx, args[args_idx]);
                    args_idx++;
                }
            } 

             if ((elements >= 1) && (!parsed)) {
                char *test_stdout = ">";
                char *test_stdin  = "<";
                char *test_args   = "-";

                if (!strncmp(token, test_stdin, 1)) {
                    parsed = 1;
                    o_stdin = my_dupe(token, 1);
                    printf("< redirection: %s\n", o_stdin);
                }

                if (!strncmp(token, test_stdout, 1)) {
                    parsed = 1;
                    oct_stdout = my_dupe(token, 1);
                    printf("> redirection: %s\n", oct_stdout);
                }

                if (!strncmp(test_args, token, 1)) {
                    parsed = 1;

                    if (args_idx < MAX_READ) {
                        args[args_idx] = my_dupe(token, 1);
                    } else { fprintf(stderr, "Too many arguments!\n"); }

                    printf("- argument at %d : %s\n", args_idx, args[args_idx]);
                    args_idx++;
                }

                // Would only get this far assuming we haven't checked any other box
                if ((!parsed)) {
                    parsed = 1;

                    if (targets_idx < MAX_READ) {
                        targets[targets_idx] = my_dupe(token, 0);
                    } else { fprintf(stderr, "Too many target files!\n"); }

                    printf("Target at %d : %s\n", targets_idx, targets[targets_idx]);
                    targets_idx++;
                }
            }

        }
        */