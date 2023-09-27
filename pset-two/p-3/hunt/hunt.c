#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // to include
#include <sys/stat.h>
#include <sys/stat.h>

int r (char *dn)
{
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
        char *cur_file = strdup(dn);
        struct stat buf;
        if (!(strcmp(".", de->d_name)) || !(strcmp("..", de->d_name))) {
            printf("Ignored\n");
        } else {
            strcat(cur_file, "/");
            strcat(cur_file, de->d_name);
            printf("%s\n", cur_file);
            lstat(cur_file, &buf);
            if ((buf.st_mode & __S_IFMT) == __S_IFDIR) {
                printf("Directory\n");
            }
            if ((buf.st_mode & __S_IFMT) == __S_IFREG) {
                printf("Regular File\n");
            }
            if ((buf.st_mode & __S_IFMT) == __S_IFLNK) {
                printf("Symlink\n");
            }
        }
    }
    if (errno)
    {
        fprintf(stderr, "Error reading directory %s - %s\n", dn, strerror(errno));
    }
    return closedir(dirp);
}

int main(int argc, char* argv[]) {
    if (argc != 3) { fprintf(stderr, "Incorrect number of arguments\n"); return -1;}
    //char *target = argv[1];
    char *start = argv[2];
    r(start);
    return 0;
}