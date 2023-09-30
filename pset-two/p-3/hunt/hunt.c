#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // to include
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>

#define BUFSIZE 4096

static char target_buf[BUFSIZE] = ""; 
static struct stat target_details;

// Store target within buffer to save time on repeated freads
int load_target_buffer(char *target) {
    FILE *read_target;
    if ((read_target = fopen(target, "r")) == NULL) {
        fprintf(stderr, "Error: %s file does not exist. Error Code: %s\n", target, strerror(errno));
        exit(-1);
    }

    while (!feof(read_target)) {
        fread(target_buf, sizeof(target_buf), 1, read_target);
    }

    if (fclose(read_target)) { 
        fprintf(stderr, "Failed to close %s - %s\n", target, strerror(errno));
        exit(-1);
    }
    return 0;
}

int compare_files(char *test) {
    // Will only work if file is a regular file and not a directory
    FILE *read_test;
    if ((read_test = fopen(test, "r")) == NULL) {
        fprintf(stderr, "DEBUG: %s links to something not a file, skipping \n", test);
        errno = 0;
        return 0;
    };

    int c = 0;
    int i = 0;
    while ((c = fgetc(read_test)) != EOF) {
        if ((!((char)c)) == target_buf[i]) {
            return 0;
        }
        i++;
    }
    
    if (fclose(read_test)) { 
        fprintf(stderr, "Failed to close %s - %s\n", test, strerror(errno));
        exit(-1);
    }
    return 1;
}

int r (char *dn, char *target)
{
    struct stat target_details;
    stat(target, &target_details);
    DIR *dirp;
    struct dirent *de;

    if(!(dirp=opendir(dn)))
    {
        fprintf(stderr, "Can not open directory %s - %s\n", dn, strerror(errno));
        return -1;
    }
    errno = 0;

    while ((de=readdir(dirp)))
    {
        char cur_path[PATH_MAX] = "";
        if (!(strcmp(".", de->d_name)) || !(strcmp("..", de->d_name))) { continue; } 
        else {
            // Update pathname
            int latest;
            for (int i = 0; i < (int)strlen(dn); i++) {
                cur_path[i] = dn[i];
                latest = i;
            }
            latest++;
            cur_path[latest] = '/';
            latest++;
            for (int i = latest; i < (latest + (int)strlen(de->d_name)); i++) {
                cur_path[i] = de->d_name[i-latest];
            }

            struct stat buf;
            lstat(cur_path, &buf);
            if (S_ISDIR(buf.st_mode)) {
                //fprintf(stderr, "Dir @ %s\n", cur_path);
                r(cur_path, target);
            }
            if (S_ISREG(buf.st_mode)) {
                //fprintf(stderr, "Regular File @ %s\n", cur_path);
                if (target_details.st_ino == buf.st_ino) {
                    fprintf(stderr, "%s\nHARDLINK TO TARGET\n", cur_path);
                } else if (compare_files(cur_path)) {
                    fprintf(stderr, "%s\nDUPLICATE OF TARGET (nlink = %ld)\n", cur_path, buf.st_nlink);
                } else { continue; }
            }
            if (S_ISLNK(buf.st_mode)) {
                //fprintf(stderr, "Link @ %s\n", cur_path);
                if ((target_details.st_dev == buf.st_dev) && (target_details.st_ino == buf.st_ino) && (target_details.st_size == buf.st_size)) {
                    printf("%s\nSYMLINK RESOLVES TO TARGET\n", cur_path); 
                } else {
                    char link_path_buf[PATH_MAX] = "";
                    struct stat tempstat;
                    stat(cur_path, &tempstat);
                    if (S_ISREG(tempstat.st_mode)) {
                        if (compare_files(cur_path)) { printf("%s\nSYMLINK RESOLVES TO DUPLICATE TARGET\n", cur_path); }
                    }
                    if (S_ISLNK(tempstat.st_mode)) {
                        ssize_t bits_read = readlink(cur_path, link_path_buf, PATH_MAX);
                        printf("\nLink: %s\n", link_path_buf);
                        if (bits_read < 0) {
                            fprintf(stderr, "Error finding symlink pathname - %s\n", strerror(errno));
                            errno = 0;
                        }
                        if (compare_files(link_path_buf)) { printf("%s\nSYMLINK RESOLVES TO DUPLICATE TARGET\n", cur_path); }
                    }
                }
            }
        }
        printf("--------------------------------------\n");
    }
    if (errno)
    {
        fprintf(stderr, "Error reading directory %s - %s\n", dn, strerror(errno));
    }
    return closedir(dirp);
}

int main(int argc, char* argv[]) {
    if (argc != 3) { fprintf(stderr, "Incorrect number of arguments\n"); return -1;}
    char *target = argv[1];
    char *start = argv[2];
    load_target_buffer(target);
    stat(target, &target_details);
    printf("DEBUG: Target is %ld bytes long, dev %ld, ino %ld\n", target_details.st_size, target_details.st_dev, target_details.st_ino);
    r(start, target);
    return 0;
}