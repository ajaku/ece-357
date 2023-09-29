#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // to include
#include <sys/stat.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <unistd.h>

#define BUFSIZE 4096

/*int compare_files(char *target, char *test) {

}*/

int r (char *dn, char *target)
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
        char cur_path[PATH_MAX] = "";
        if (!(strcmp(".", de->d_name)) || !(strcmp("..", de->d_name))) {
            continue;
        } else {
            struct stat buf;
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
            lstat(cur_path, &buf); // using lstat to print information about file
            if ((buf.st_mode & __S_IFMT) == __S_IFDIR) {
                printf("Current Path: %s\nDirectory\n", cur_path);
                r(cur_path, target);
            }
            if ((buf.st_mode & __S_IFMT) == __S_IFREG) {
                printf("Current Path: %s\nRegular File\n", cur_path);
                printf("File Size: %ld\n", buf.st_size);
                printf("Inode Number: %ld\n", buf.st_ino);
            }
            if ((buf.st_mode & __S_IFMT) == __S_IFLNK) {
                // need to run stat to determine information information about file that the symlink points to
                printf("Current Path: %s\nSymlink\n", cur_path);
                struct stat sym_buf;
                stat(cur_path, &sym_buf);
                if ((sym_buf.st_mode & __S_IFMT) == __S_IFREG) {
                    printf("File Type: Regular File\n");
                    printf("File Size: %ld\n", sym_buf.st_size);
                    printf("Inode Number: %ld\n", sym_buf.st_ino);
                    char link_path_buf[PATH_MAX];
                    readlink(cur_path, link_path_buf, PATH_MAX);
                    printf("Link to Path: %s\n", link_path_buf);
                    char contents_of_link[BUFSIZE];
                    int bits_read = read(link_path_buf, contents_of_link, BUFSIZE);
                    printf("Bits read: %d\n", bits_read);
                    printf("Contents of Link: %s\n", contents_of_link);
                } else {
                    printf("Not a regular file so ignoring\n");
                }
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
    char *target = argv[1];
    char *start = argv[2];
    r(start, target);
    return 0;
}