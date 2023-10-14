#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> // strerr
#define _GNU_SOURCE
#include <unistd.h> // fork
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_READ 4096
// Hoping we user doesn't have an SH filer larger than this
#define MAX_SH_LENGTH 100000

// Stores current working directory
char cwd[MAX_READ];
// Store current exit status
int exit_status = 0;
// Stores shell for sh parsing
char *shell = NULL;

// Struct that contains parsed content from the input
typedef struct parsed_s {
    char *redirections[5];
    //o_stdin, *oct_stdout, *oct_stderr, *oca_stdout, *oca_stderr;
    int args_idx;
    int eof_recv;
    char *args[MAX_READ]; 
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
    char *my_str = (char *)malloc((my_size + 1)*sizeof(char));

    for (int i = start; i < size; i++) {
        *(my_str + i - start) = *(str + i);
    }

    *(my_str + size) = '\0';

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

int parse_input(char *input, parsed_t *params, int script) {
    char *token;

    if ((token = strtok(input, " \n")) == NULL) {
        // If reading from a script, can't recieve CTRL + D so it must be \n
        if (script) {
            return 1;
        }

        // When null can either be because enter (new line) or CTRL + D. 
        // Need to distinguish between the two cases
        int c;
        if ((c = getchar()) == EOF) {
            return 2;
        } else { fprintf(stderr, "Error tokenizing input %s - %s\n", input, strerror(errno)); }

        return 1;
    }

    params->args[params->args_idx] = my_dupe(token, 0);
    params->args_idx++;

    // Want to skip lines with #
    if (!(strncmp(token, "#", 1))) {
        return 1;
    }

    while ((token = strtok(NULL, " \n")) != NULL) {
        int elements = strlen(token);
        int redirect = 0;

        // Check for 2>>
        if ((elements >= 3) && (!redirect)) {
            char test_stderr[3] = "2>>";
            if (!strncmp(test_stderr, token, 3)) { 
                redirect = 1;
                params->redirections[4] = my_dupe(token, 3);
                //printf("2>> redirection: %s\n", params->oca_stderr);
            }
        } 

        if ((elements >= 2) && (!redirect)) {
            char test_stdout[2] = ">>";
            char test_stderr[2] = "2>";

            if (!strncmp(test_stderr, token, 2)) { 
                redirect = 1;
                params->redirections[2] = my_dupe(token, 2);
                //printf("2> redirection: %s\n", params->oct_stderr);
            }

            if (!strncmp(test_stdout, token, 2)) { 
                redirect = 1;
                params->redirections[3] = my_dupe(token, 2);
                //printf(">> redirection: %s\n", params->oca_stdout);
            }
        } 

         if ((elements >= 1) && (!redirect)) {
            char *test_stdout = ">";
            char *test_stdin  = "<";

            if (!strncmp(token, test_stdin, 1)) {
                redirect = 1;
                params->redirections[0] = my_dupe(token, 1);
                //printf("< redirection: %s\n", params->o_stdin);
            }

            if (!strncmp(token, test_stdout, 1)) {
                redirect = 1;
                params->redirections[1] = my_dupe(token, 1);
                //printf("> redirection: %s\n", params->oct_stdout);
            }

            // Would only get this far assuming we haven't checked any other box
            if ((!redirect)) {
                // need to leave room for final NULL
                if (params->args_idx < MAX_READ-1) {
                    params->args[params->args_idx] = my_dupe(token, 0);
                } else { fprintf(stderr, "Too many target arguments!\n"); }

                params->args_idx++;
            }
        }
    }
    return 0;
}

// Partly Inspired by: https://stackoverflow.com/questions/52939356/redirecting-i-o-in-a-custom-shell-program-written-in-c
int handle_redir(parsed_t *flags) {
    // o_stdin, *oct_stdout, *oct_stderr, *oca_stdout, *oca_stderr;
    // 0         1            2            3            4
    for (int i = 0; i < 5; i++) {
        if(flags->redirections[i] != NULL) {
            // open stdin
            int fd;
            if (!i) {
                if ((fd = open(flags->redirections[i], O_RDONLY, 0644 )) < 0) {
                    fprintf(stderr, "Error opening %s - %s\n", flags->redirections[i], strerror(errno));
                    return 1;
                }
                dup2(fd, STDIN_FILENO);
            }

            if ((i == 1) || i == 2) {
                if ((fd = open(flags->redirections[i], (O_WRONLY | O_CREAT | O_TRUNC), 0664)) < 0) {
                    fprintf(stderr, "Error opening %s - %s\n", flags->redirections[i], strerror(errno));
                    return 1;
                }
                if (i == 1) {
                    dup2(fd, STDOUT_FILENO);
                } else { dup2(fd, STDERR_FILENO); }
            }

            if ((i == 3) || (i == 4)) {
                if ((fd = open(flags->redirections[i], (O_WRONLY | O_CREAT | O_APPEND), 0664)) < 0) {
                    fprintf(stderr, "Error opening %s - %s\n", flags->redirections[i], strerror(errno));
                    return 1;
                }
                if (i == 3) {
                    dup2(fd, STDOUT_FILENO);
                } else { dup2(fd, STDERR_FILENO); }
            }
            close(fd);
        }
    }
    return 0;
}

// Partly inspired by: https://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html
void exec_cmd(char **argv, parsed_t *flags, int EXIT_FLAG) {
    pid_t pid;
    int   status;
    pid_t child_pid; 
    struct rusage ru;

    if (EXIT_FLAG >= 0) {
        if (EXIT_FLAG == 0) {
            exit(exit_status);
        } 
        if (EXIT_FLAG > 0) {
            exit(EXIT_FLAG);
        }
    } 

    switch (pid = fork()) {
        case -1:
            fprintf(stderr, "Fork failed!\n"); exit(1);
            break;

        case 0:
            child_pid = getpid();

            if ((exit_status = handle_redir(flags)) != 1) {
                child_pid = getpid();
                
                if (execvp(argv[0], argv) < 0) {
                    fprintf(stderr, "Exec failed with argument %s - %s\n", argv[0], strerror(errno));
                    exit(127);
                } 
            } 
            fprintf(stderr, "Exit status %d\n", exit_status);
            break;

        default:
            int rep_time = 0;
            if(wait3(&status, 0, &ru) == -1) {
                fprintf(stderr, "wait3 failed\n");
            } else {
                rep_time = 1;
            }

            if (WIFEXITED(status)) {

                if (WEXITSTATUS(status) == 0) {
                    fprintf(stderr, "Child process %d exited normally\n", child_pid);
                } else { fprintf(stderr, "Child process %d failed with return value of %d\n", child_pid, WEXITSTATUS(status) + exit_status); }

                /* Attempting to capture correct exit value
                 * Confused a bit about the difference between +127 and just standard return values?
                 */
                if (WEXITSTATUS(status == 0)) {
                    exit_status += WEXITSTATUS(status) ; 
                } else {
                    exit_status = WEXITSTATUS(status) ; 
                }
            } 

            if (WIFSIGNALED(status)) {

                fprintf(stderr, "Child process %d failed with return value of %d\n", child_pid, WTERMSIG(status));
                exit_status = WTERMSIG(status); 

            }

            if (rep_time) {
                fprintf(stderr, "Real: %ld.%.6lds User: %ld.%.6lds Sys: %ld.%.6lds\n", (ru.ru_utime.tv_sec + ru.ru_stime.tv_sec), (ru.ru_utime.tv_usec + ru.ru_stime.tv_usec),
                                                                      ru.ru_utime.tv_sec, ru.ru_utime.tv_usec,
                                                                      ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);
            }
            break;
    }
}

void check_builtin (parsed_t *my_params) {
            my_params->args[my_params->args_idx+1] = NULL;
            if(!(my_params->args[0])) {

                if(my_params->eof_recv) {
                    exec_cmd(NULL, NULL, 0);
                }

            } else if (!(strcmp(my_params->args[0], "pwd"))) {

                my_pwd(cwd);
                printf("%s\n", cwd);

            } else if (!(strcmp(my_params->args[0], "cd"))) {

                my_cd(my_params->args[1]);

            } else if (!(strcmp(my_params->args[0], "exit"))) {

                if((my_params->args[1] != NULL)) {
                    // Inspired by: https://sentry.io/answers/char-to-int-in-c-and-cpp/
                    char *output;
                    long exit_val = strtol(my_params->args[1], &output, 0);
                    exec_cmd(NULL, NULL, exit_val);

                } else { exec_cmd(NULL, NULL, 1); }

            } else { exec_cmd(my_params->args, my_params, -1); }
}

void free_all(parsed_t *my_params) {

}

int main(int argc, char* argv[]) {

    errno = 0;

    if (argc > 2) { fprintf(stderr, "Too many inputs\n"); return -1; }

    FILE *script = NULL;

    if (argc == 2) {

        if(!(script = fopen(argv[1], "r"))) {
            fprintf(stderr, "Error opening input script %s - %s\n", argv[1], strerror(errno));
            return -1;
        }

        size_t len = 0;
        char *input[MAX_SH_LENGTH];
        int real_idx = 0;
        // Inspired by: https://stackoverflow.com/questions/68996815/saving-getline-output-to-an-external-array
        /* Pre-populate an array with input lines from shell script
         * Motivation was that strangely, when using getline, if an error occured
         * on a due to a command, it would be appended to the final line of getline,
         * causing getline to go back to a previous point
         */
        for (int i = 0; i < MAX_SH_LENGTH; i++) {
            len = 0;
            if ((getline(&input[i], &len, script)) == -1) {
                break;
            }
            real_idx++;
        }
        // Would have been used to perform #!./myshell operation
        shell = input[0];
        /* When running a shell script we were instructed to place #!./myshell
         * as the interpreter, yet I'm not entirely sure how to then use that
         * to find the binaries properly, so that was not implemented.
         * (Would've asked questions but unfortuantely this is already late, sorry)
         */
        for (int i = 1; i <= real_idx; i++) {
            parsed_t my_params = { .args_idx = 0 };

            int r_parse = parse_input(input[i], &my_params, 1);


            // r_parse = 0 => CTRL + D has been received so exit
            // r_parse = 1 => Comment or empty so skip
            if (r_parse == 1) { goto skip_script; }
            if (r_parse == 2) { my_params.eof_recv = 1; }

            check_builtin(&my_params);

            skip_script:
                continue;

            i++;

            for (int j = 0; j < my_params.args_idx; j++ ) { free(my_params.args[j]); }
            for (int k = 0; k < 5 ; k++ ) { free(my_params.redirections[k]); }
        }
        // Would be done with script at this point
        for (int i = 0; i < real_idx; i++) {
            free(input[i]);
        }
    } else {
        while(1) {
            size_t len = 0;
            ssize_t read;
            char *input = NULL; // Remember to free!
            read = getline(&input, &len, stdin);

            parsed_t my_params = { .args_idx = 0 };

            int r_parse = parse_input(input, &my_params, 0);

            // r_parse = 0 => CTRL + D has been received so exit
            // r_parse = 1 => Comment or empty so skip
            if (r_parse == 1) { goto skip_stdin; }
            if (r_parse == 2) { my_params.eof_recv = 1; }

            check_builtin(&my_params); 

            skip_stdin:
                continue;

            for (int i = 0; i < my_params.args_idx; i++ ) { free(my_params.args[i]); }
            for (int k = 0; k < 5 ; k++ ) { free(my_params.redirections[k]); }
            free(input);
        }
    }
    return exit_status;
}