#include <stdio.h>
#include <unistd.h>
#include "my_stdio.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define FILE_SIZE 4096

int main(int argc, char *argv[]) {
	int opt;
	char *outfile = NULL;
	while ((opt = getopt(argc, argv, "o")) != -1) {
		switch (opt) {
			case 'o':
				outfile = argv[2]; // Handles outfile
				break;
		}
	}

	char buf[FILE_SIZE];
	int read_fd;
	int write_fd;

	// Open the write file descriptor
	if (access(argv[2], F_OK) == 0) {
		if (outfile) {
        	write_fd = open(argv[2], O_WRONLY | O_TRUNC);
			printf("Opening in truncation mode\n");
		} else { 
        	write_fd = STDOUT_FILENO;
		}
		printf("File exists: %d\n", write_fd);
	}
 	else {
		if (outfile) {
        	write_fd = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0777);
		} else { 
        	write_fd = STDOUT_FILENO;
		}
		printf("Created file: %d\n", write_fd);
	}
	if (write_fd == -1) {
		printf("Error opening file for writing: errno %d - %s\n", errno, strerror(errno));
		return -1;
	}
	
	int i = 1;
	if (outfile) { i = 3; } 

	// Open the read file descriptor
	while (i < argc) {	
		if (*argv[i] == '-') {
			read_fd = STDIN_FILENO; 
		} else {
			read_fd = open(argv[i], O_RDONLY);
			if (read_fd == -1) {
				printf("Error opening file for reading: errno %d - %s\n", errno, strerror(errno));
				return -1;
			}
		}
		int j = 1;
		while (j != 0) {
			j = read(read_fd, buf, FILE_SIZE);
			if (j == -1) {
				printf("Error reading file: errno %d - %s\n", errno, strerror(errno));
				if (close(read_fd) == -1) {
					printf("Error closing file: errno %d - %s\n", errno, strerror(errno));
				}
				if (close(write_fd) == -1) {
					printf("Error closing file: errno %d - %s\n", errno, strerror(errno));
				}
				return -1;
			}			// Write out contents from buffer into write file
	 	    int k = write(write_fd, buf, j);
			if (k == -1) {
				printf("Error writing to file: errno %d - %s\n", errno, strerror(errno));
				if (close(read_fd) == -1) {
					printf("Error closing file: errno %d - %s\n", errno, strerror(errno));
				}
				if (close(write_fd) == -1) {
					printf("Error closing file: errno %d - %s\n", errno, strerror(errno));
				}
			return -1;
			}
		}
		i++;
	}

	int write_close = close(write_fd);
	if (write_close == -1) {
		printf("Error closing file: errno %d - %s\n", errno, strerror(errno));
		return -1;
	}

	int read_close = close(read_fd);	
	if (read_close == -1) {
		printf("Error closing file: errno %d - %s\n", errno, strerror(errno));
		return -1;
	}
	return 0;
}