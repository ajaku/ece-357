#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

// Inspired by: https://www.youtube.com/watch?v=6xbLgZpOBi8&t=630s

int main(int argc, char* argv[]) {
    errno = 0;

    if (argc > 2)  { fprintf(stderr, "Too many input arguments\n"); return 1; }

    int pid[3];
    int pfds[2][2];

    int status[3];

    /*
     * pfds[0][0] - Wordsearch Reader   fd[0][1] - Wordgen Writer 
     * pfds[1][0] - Pager Reader        fd[1][1] - Wordsearch Writer 
     * 
     */


    for (int i = 0; i < 2; i++) {
        if (pipe(pfds[i]) < 0) {
            perror("Pipe failed");
        }
    }

    // wordgen -> only writes
    // wordsearch -> reads from wordgen, writes to pager
    // pager -> reads from wordsearch, writes to stdout

    pid[0] = fork();
    if (pid[0] == -1) { perror("Fork Failed"); return 1; }
    if (pid[0] ==  0) {

        close(pfds[0][0]);
        //close(pfds[0][1]);
        close(pfds[1][0]);
        close(pfds[1][1]);

        // Redirect stdout to the writer fd
        dup2(pfds[0][1], STDOUT_FILENO);
        close(pfds[0][1]);

        if (argv[1] == NULL) {
            execl("./wordgen", "./wordgen", NULL);
        } else {
            execl("./wordgen", "./wordgen", argv[1], NULL);
        }
        fprintf(stderr, "\n\nWORDGEN EXITED\n");
        exit(0);
    }

    pid[1] = fork();
    if (pid[1] == -1) { perror("Fork Failed"); return 1; }
    if (pid[1] ==  0) {

        //close(pfds[0][0]);
        close(pfds[0][1]);
        close(pfds[1][0]);
        //close(pfds[1][1]);

        dup2(pfds[0][0], STDIN_FILENO);
        close(pfds[0][0]);
        dup2(pfds[1][1], STDOUT_FILENO);
        close(pfds[1][1]);
        execl("./wordsearch", "./wordsearch", "shorter_words.txt", NULL);
        fprintf(stderr, "\n\nWORDSEARCH EXITED\n");
        exit(0);

    }

    pid[2] = fork();
    if (pid[2] == -1) { perror("Fork Failed"); return 1; }
    if (pid[2] ==  0) {

        close(pfds[0][0]);
        close(pfds[0][1]);
        //close(pfds[1][0]);
        close(pfds[1][1]);

        dup2(pfds[1][0], STDIN_FILENO);
        close(pfds[1][0]);
        execl("./pager", "./pager", NULL);
        exit(0);
    }

    close(pfds[0][0]);
    close(pfds[0][1]);
    close(pfds[1][0]);
    close(pfds[1][1]);
    
    for (int i = 0; i < 3; i++) {
        waitpid(pid[i], &status[i], 0);
        if (WIFEXITED(status[i])) {
            fprintf(stderr, "Child %d exited with %d\n", pid[i], WEXITSTATUS(status[i]));
        }
        if (WIFSIGNALED(status[i])) {
            fprintf(stderr, "Child %d exited with %d\n", pid[i], WTERMSIG(status[i]));
        }
    }

    /*
    waitpid(pid[0], &status[0], 0);
    if (WIFEXITED(status[0])) {
        fprintf(stderr, "Child (wordgen) %d exited with %d\n", pid[0], WEXITSTATUS(status[0]));
    }
    if (WIFSIGNALED(status[0])) {
        fprintf(stderr, "Child (wordgen) %d exited with %d\n", pid[0], WTERMSIG(status[0]));
    }
    waitpid(pid[1], &status[1], 0);
    if (WIFEXITED(status[1])) {
        fprintf(stderr, "Child (wordsearch) %d exited with %d\n", pid[1], WEXITSTATUS(status[1]));
    }
    if (WIFSIGNALED(status[1])) {
        fprintf(stderr, "Child (wordsearch) %d exited with %d\n", pid[1], WTERMSIG(status[1]));
    }
    waitpid(pid[2], &status[2], 0);
    if (WIFEXITED(status[2])) {
        fprintf(stderr, "Child (pager) %d exited with %d\n", pid[2], WEXITSTATUS(status[2]));
    }
    */

    return 0;
}
