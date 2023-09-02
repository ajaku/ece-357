#include <stdio.h>
#include <unistd.h>
#include "my_stdio.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

/* Only two real cases
 * 1. Specify output file
 * 2. Don't and assume standard input
 *
 * Read from inputs '-' means standard input until EOF
 * Can have multiple - or inputs from the same file
 *
 * Write the information into the outfile or standard out
 *
 */
int main(int argc, char *argv[]) {
	int opt;
	char *outfile = NULL;
	while ((opt = getopt(argc, argv, "o")) != -1) {
		switch (opt) {
			case 'o':
				outfile = *(argv + 2); // Handles outfile
				break;
		}
	}

	for (int i = 0; i < 7; i++) {
		printf("%s", *(argv + i));
	}
	return 0;
}
