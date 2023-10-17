#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>

/* Purpose of Program: Handle a signal >32 and <32
 * Note the difference between the two
 * >32 should have as many deliveries as signals were sent
 * <32 should only have one instance
 * 
 * Partial Sources:
 * https://www.youtube.com/watch?v=94URLRsjqMQ
 * https://www.youtube.com/watch?v=up4-42var6o
 */

void handler(int s) {
    fprintf(stderr, "Got a signal %d\n", s);
}

int main() {
    pid_t pid[3];
    int status;
    pid_t child_pid[3];

    // Create three seperate PIDs
    // child_pid[0] - will be reciever of signals (will have handler)
    // child_pid[1] - will send >32 signals multiple times
    // child_pid[2] - will send <32 signals multiple times
    switch (pid[0] = fork()) {
        case -1:
            fprintf(stderr, "Fork failed\n"); exit(1);
            break;
        
        case 0:
            child_pid[0] = getpid();
            pid[0] = getppid();

            signal(SIGINT, handler);

            fprintf(stderr, "Child 1 PID: %d\n", child_pid[0]); 
            fprintf(stderr, "Parent 1 PID: %d\n", pid[0]);

            break;

        default:
            switch (pid[1] = fork()) {
                case -1:
                    fprintf(stderr, "Fork failed\n"); exit(1);
                    break;
        
                case 0:
                    child_pid[1] = getpid();
                    pid[1] = getppid();
                    fprintf(stderr, "Child 2 PID: %d\n", child_pid[1]); 
                    fprintf(stderr, "Parent 2 PID: %d\n", pid[1]);

                    // Will send SIGINT to Child 1
                    kill(child_pid[0], SIGINT);
                    break;

                default:
                    switch (pid[2] = fork()) {
                        case -1:
                            fprintf(stderr, "Fork failed\n"); exit(1);
                            break;
        
                        case 0:
                            child_pid[2] = getpid();
                            pid[2] = getppid();
                            fprintf(stderr, "Child 3 PID: %d\n", child_pid[2]); 
                            fprintf(stderr, "Parent 3 PID: %d\n", pid[2]);
                            kill(child_pid[0], SIGINT);
                            break;

                        default:
                            break;
                }
            }
            break;
    }

    while (wait(NULL) != -1 || errno !=  ECHILD) {
        printf("Waiting for child to exit\n");
    } 

    return 0;
}