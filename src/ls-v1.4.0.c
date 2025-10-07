#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <getopt.h>
#include <errno.h>

#define SPACING 2

int compare(const void *a, const void *b) {
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb);
}

int get_terminal_width() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        return 80;
    }
    return ws.ws_col;
}

void print_permissions(mode_t mode) {
    char perms[11] = "----------";
    if (S_ISDIR(mode)) perms[0] = 'd';
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

void list_long(char **filenames, int count, const char *path) {
    char fullpath[1024];
    struct stat st;

    for (int i = 0; i < count; i++) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filenames[i]);
        if (stat(fullpath, &st) == -1) {
            perror("stat failed");
            continue;
        }

        print_permissions(st.st_mode);
        printf("%2ld ", (long)st.st_nlink);

        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        printf("%s %s ", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

        printf("%6ld ", (long)st.st_size);

        char *time_str = ctime(&st.st_mtime);
        time_str[strlen(time_str) - 1] = '\0';
        printf("%s ", time_str);

        printf("%s\n", filenames[i]);
    }
}

void list_horizontal(char **filenames, int count, int max_len) {
    int term_width = get_terminal_width();
    int col_width = max_len + SPACING;
    int curr_width = 0;

    for (int i = 0; i < count; i++) {
        if (curr_width + col_width > term_width) {
            printf("\n");
            curr_width = 0;
        }
        printf("%-*s", col_width, filenames[i]);
        curr_width += col_width;
    }
    printf("\n");
}

void list_vertical(char **filenames, int count, int max_len) {
    int term_width = get_terminal_width();
    int col_width = max_len + SPACING;
    int cols = term_width / col_width;
    if (cols == 0) cols = 1;
    int rows = (count + cols - 1) / cols;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int idx = col * rows + row;
            if (idx < count)
                printf("%-*s", col_width, filenames[idx]);
        }
        printf("\n");
    }
}

void list_files(const char *path, int mode_long, int mode_horizontal) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    char **filenames = NULL;
    size_t count = 0, capacity = 16;
    size_t max_len = 0;
    filenames = malloc(capacity * sizeof(char *));
    if (!filenames) {
        perror("malloc failed");
        closedir(dir);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        if (count == capacity) {
            capacity *= 2;
            filenames = realloc(filenames, capacity * sizeof(char *));
            if (!filenames) {
                perror("realloc failed");
                closedir(dir);
                return;
            }
        }

        filenames[count] = strdup(entry->d_name);
        if (!filenames[count]) {
            perror("strdup failed");
            closedir(dir);
            return;
        }

        size_t len = strlen(entry->d_name);
        if (len > max_len)
            max_len = len;

        count++;
    }
    closedir(dir);

    // Sort alphabetically
    qsort(filenames, count, sizeof(char *), compare);

    if (mode_long)
        list_long(filenames, count, path);
    else if (mode_horizontal)
        list_horizontal(filenames, count, max_len);
    else
        list_vertical(filenames, count, max_len);

    for (size_t i = 0; i < count; i++)
        free(filenames[i]);
    free(filenames);
}

int main(int argc, char *argv[]) {
    int opt;
    int mode_long = 0, mode_horizontal = 0;
    const char *target_dir = ".";

    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                mode_long = 1;
                break;
            case 'x':
                mode_horizontal = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc)
        target_dir = argv[optind];

    list_files(target_dir, mode_long, mode_horizontal);
    return 0;
}

