#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

// Partial Source: Jacob Sorber's Mapping Files into Memory in C

int main(int argc, char** argv) {
	if (argc < 4) { printf("Not enough inputs\n"); return 1; }

	char *target = argv[1];
	char *rep    = argv[2];

	int target_size = strlen(target);
	int rep_size    = strlen(rep);
	
	if (rep_size != target_size) { printf("Target and replacement must be same size\n"); return 1; }
	
	int  i = 3;
	while (i != argc) {

		int fd;
		if ((fd = open(argv[i], O_RDWR)) < 0) {
			perror("Failed to open file");
			return 1;
		}
		
		struct stat sb;
		if (fstat(fd, &sb) == -1) {
			perror("Failed to perform fstat");
			return 1;
		}

		// Empty file should just return 0
		if (!sb.st_size) { return 0; }

		char *map; 
		// sb.st_size is the size of input file (bytes)
		// PROT_READ and PROT_WRITE so that 
		// MAP_SHARED to write back to file
		// fd of opened file
		// No offset
		if ((map = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
			perror("mmap failed");
			return 1;
		}


		// Will utilize strstr to find first occurance of substring n times
		char *p;

		// Substring does not exist within map
		if ((p = strstr(map, target)) == NULL) { return 0; }

		// first j characters of p will be substring
		while (p) { 
			for (int j = 0; j < target_size; j++) {
				*(p + j) = *(rep + j); 
			}
			// Am assuming we care about case, if we didn't would have done:
			// p = strcasestr(map, target); to ignore case
			p = strstr(map, target);
		}

		if (close(fd) == -1) { 
			perror("Failed to close file"); 
			return 1; 
		}

		i++;	
	}

	return 0;
}
