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

#define COLOR_RESET     "\033[0m"
#define COLOR_BLUE      "\033[0;34m"
#define COLOR_GREEN     "\033[0;32m"
#define COLOR_MAGENTA   "\033[0;35m"
#define COLOR_RED       "\033[0;31m"
#define COLOR_REVERSE   "\033[7m"

int compare(const void *a, const void *b) {
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb);
}

int get_terminal_width() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
        return 80;
    return ws.ws_col;
}

const char* get_color(const char *name, mode_t mode) {
    if (S_ISDIR(mode)) return COLOR_BLUE;
    if (S_ISLNK(mode)) return COLOR_MAGENTA;
    if (S_ISCHR(mode) || S_ISBLK(mode) || S_ISFIFO(mode) || S_ISSOCK(mode)) return COLOR_REVERSE;
    if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return COLOR_GREEN;
    if (strstr(name, ".zip") || strstr(name, ".tar") || strstr(name, ".gz")) return COLOR_RED;
    return COLOR_RESET;
}

void print_colored(const char *name, const char *path) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, name);

    struct stat st;
    if (lstat(fullpath, &st) == -1) {
        perror("lstat failed");
        printf("%s ", name); // fallback
        return;
    }

    const char *color = get_color(name, st.st_mode);
    printf("%s%s%s", color, name, COLOR_RESET);
}

void list_horizontal(char **files, int count, int max_len, const char *path) {
    int term_width = get_terminal_width();
    int col_width = max_len + SPACING;
    int curr_width = 0;

    for (int i = 0; i < count; i++) {
        if (curr_width + col_width > term_width) {
            printf("\n");
            curr_width = 0;
        }
        print_colored(files[i], path);
        printf("%*s", col_width - (int)strlen(files[i]), "");
        curr_width += col_width;
    }
    printf("\n");
}

void list_vertical(char **files, int count, int max_len, const char *path) {
    int term_width = get_terminal_width();
    int col_width = max_len + SPACING;
    int cols = term_width / col_width;
    if (cols == 0) cols = 1;
    int rows = (count + cols - 1) / cols;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int idx = col * rows + row;
            if (idx < count) {
                print_colored(files[idx], path);
                printf("%*s", col_width - (int)strlen(files[idx]), "");
            }
        }
        printf("\n");
    }
}

void list_files(const char *path, int horizontal_flag) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    struct dirent *entry;
    char **files = NULL;
    size_t count = 0, capacity = 16;
    size_t max_len = 0;

    files = malloc(capacity * sizeof(char *));
    if (!files) {
        perror("malloc failed");
        closedir(dir);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        if (count == capacity) {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(char *));
            if (!files) {
                perror("realloc failed");
                closedir(dir);
                return;
            }
        }

        files[count] = strdup(entry->d_name);
        if (!files[count]) {
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

    qsort(files, count, sizeof(char *), compare);

    if (horizontal_flag)
        list_horizontal(files, count, max_len, path);
    else
        list_vertical(files, count, max_len, path);

    for (size_t i = 0; i < count; i++)
        free(files[i]);
    free(files);
}

int main(int argc, char *argv[]) {
    int opt;
    int horizontal = 0;
    const char *target_dir = ".";

    while ((opt = getopt(argc, argv, "x")) != -1) {
        switch (opt) {
            case 'x':
                horizontal = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-x] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc)
        target_dir = argv[optind];

    list_files(target_dir, horizontal);
    return 0;
}

