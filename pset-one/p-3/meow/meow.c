#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define FILE_SIZE 4096

int close_fds(int read_fd, char *infile, int write_fd, char *outfile) {
	if (close(read_fd) == -1) {
		fprintf(stderr, "Error closing %s: errno %d - %s\n", infile, errno, strerror(errno));
		return -1;
	}
	if (close(write_fd) == -1) {
		fprintf(stderr, "Error closing %s: errno %d - %s\n", outfile, errno, strerror(errno));
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	int opt;
	char *outfile = NULL;
	while ((opt = getopt(argc, argv, "o")) != -1) {
		switch (opt) {
			case 'o':
				outfile = argv[2]; // Sets outfile to argument of -o flag
				break;
		}
	}

	char buf[FILE_SIZE];
	int read_fd;
	int write_fd;
	char *infile;

	// Open the write file descriptor
	// If arg1 in ./meow arg1 exists 
	if (access(argv[2], F_OK) == 0) {
		// if -o flag was specified
		if (outfile) {
        	write_fd = open(argv[2], O_WRONLY | O_TRUNC);
		} else { 
        	write_fd = STDOUT_FILENO;
			outfile = "<standard out>"; 
		}
	}
 	else {
		if (outfile) {
        	write_fd = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0777);
		} else { 
        	write_fd = STDOUT_FILENO;
			outfile = "<standard out>";
		}
	}

	if (write_fd == -1) {
		fprintf(stderr, "Error opening %s for writing: errno %d - %s\n", outfile, errno, strerror(errno));
		return -1;
	}
	
	int i = 1;
	if (outfile == argv[2]) { i = 3; } 

	// Open each read file descriptor
	while (i < argc) {	
		// Determine input to be standard in or file
		char *infile;
		if (*argv[i] == '-') {
			read_fd = STDIN_FILENO; 
			infile = "<standard input>";
		} else {
			read_fd = open(argv[i], O_RDONLY);
			infile = argv[i];
			if (read_fd == -1) {
				fprintf(stderr, "Error opening %s for reading: errno %d - %s\n", infile, errno, strerror(errno));
				return -1;
			}
		}
		// Read all the bytes present within the input source
		int j = 1;
		int total_bytes_w;
		int total_new_lines;
		while (j != 0) {
			j = read(read_fd, buf, FILE_SIZE);
			if (j == -1) {
				fprintf(stderr, "Error reading %s: errno %d - %s\n", infile, errno, strerror(errno));
				if (close_fds(read_fd, infile, write_fd, outfile)) { return -1; }
			}			
			for (int i = 0; i < j; i++) {
				if (buf[i] == '\n') { total_new_lines++; }
			}
			// Write out contents from read buffer into write file
	 	    int k = write(write_fd, buf, j);
			total_bytes_w += k;
			if (k < j) {
				char *loc = buf + (j-k);
				char temp[j-k]; 
				strcpy(temp, loc); 
				int n = write(write_fd, temp, j-k);
				if (n == -1) { 
					fprintf(stderr, "Error partial-writing to %s: errno %d - %s\n", outfile, errno, strerror(errno));
					if (close_fds(read_fd, infile, write_fd, outfile)) { return -1; }
				}
			}
			if (k == -1) {
				fprintf(stderr, "Error writing to %s: errno %d - %s\n", outfile, errno, strerror(errno));
				if (close_fds(read_fd, infile, write_fd, outfile)) { return -1; }
			}
		}
		// Append new line after reading from a file that isn't STDIN (to match cat's behavior)
		// Conditions for a new line at end: Writing to stdout, attempting to read from stdin but it is not the first time reading from stdin.
		/*if ((outfile == "<standard out>") && (read_fd == STDIN_FILENO) && (i > 2)) {
			int n = write(write_fd, "\n", 1);
			if (n == -1) { 
				fprintf(stderr, "Error writing to %s: errno %d - %s\n", outfile, errno, strerror(errno));
				if (close_fds(read_fd, infile, write_fd, outfile)) { return -1; }
			}
			total_new_lines++;
		}*/
		fprintf(stderr, "Input File: %s - Bytes Transferred: %d - Number of Lines: %d\n", infile, total_bytes_w, total_new_lines);
		i++;
	}
	return (close_fds(read_fd, infile, write_fd, outfile));
}