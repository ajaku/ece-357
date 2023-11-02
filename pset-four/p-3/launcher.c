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

    int pfds[2][2];

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

    int pid1 = fork();
    if (pid1 == -1) { perror("Fork Failed"); return 1; }
    if (pid1 ==  0) {

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
    }

    int pid2 = fork();
    if (pid2 == -1) { perror("Fork Failed"); return 1; }
    if (pid2 ==  0) {

        //close(pfds[0][0]);
        close(pfds[0][1]);
        close(pfds[1][0]);
        //close(pfds[1][1]);

        dup2(pfds[0][0], STDIN_FILENO);
        close(pfds[0][0]);
        dup2(pfds[1][1], STDOUT_FILENO);
        close(pfds[1][1]);
        execl("./wordsearch", "./wordsearch", "shorter_words.txt", NULL);

    }

    int pid3 = fork();
    if (pid3 == -1) { perror("Fork Failed"); return 1; }
    if (pid3 ==  0) {

        close(pfds[0][0]);
        close(pfds[0][1]);
        //close(pfds[1][0]);
        close(pfds[1][1]);

        dup2(pfds[1][0], STDIN_FILENO);
        close(pfds[1][0]);
        execl("./pager", "./pager", NULL);
    }

    close(pfds[0][0]);
    close(pfds[0][1]);
    close(pfds[1][0]);
    close(pfds[1][1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return 0;
}
