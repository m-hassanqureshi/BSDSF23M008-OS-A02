#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

void print_permissions(mode_t mode) {
    char perms[11] = "----------";

    if (S_ISDIR(mode))  perms[0] = 'd';
    else if (S_ISLNK(mode)) perms[0] = 'l';
    else if (S_ISCHR(mode)) perms[0] = 'c';
    else if (S_ISBLK(mode)) perms[0] = 'b';
    else if (S_ISSOCK(mode)) perms[0] = 's';
    else if (S_ISFIFO(mode)) perms[0] = 'p';

    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = 'x';
    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = 'x';
    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = 'x';

    printf("%s ", perms);
}

void list_long(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    struct dirent *entry;
    char fullpath[1024];
    struct stat st;

    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files
        if (entry->d_name[0] == '.')
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        if (stat(fullpath, &st) == -1) {
            perror("stat failed");
            continue;
        }

        print_permissions(st.st_mode);
        printf("%2ld ", (long)st.st_nlink);

        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        printf("%s %s ", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

        printf("%6ld ", (long)st.st_size);

        char *mod_time = ctime(&st.st_mtime);
        mod_time[strlen(mod_time) - 1] = '\0'; // remove newline
        printf("%s ", mod_time);

        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

void list_simple(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.')
            printf("%s  ", entry->d_name);
    }
    printf("\n");
    closedir(dir);
}

int main(int argc, char *argv[]) {
    int opt;
    int long_format = 0;
    const char *target_dir = ".";

    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l':
                long_format = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        target_dir = argv[optind];
    }

    if (long_format)
        list_long(target_dir);
    else
        list_simple(target_dir);

    return 0;
}

