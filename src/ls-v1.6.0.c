#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#define SPACING 2
#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_REVERSE "\033[7m"

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

const char *get_color(const char *name, mode_t mode) {
    if (S_ISDIR(mode)) return COLOR_BLUE;
    if (S_ISLNK(mode)) return COLOR_MAGENTA;
    if (S_ISCHR(mode) || S_ISBLK(mode) || S_ISFIFO(mode) || S_ISSOCK(mode)) return COLOR_REVERSE;
    if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return COLOR_GREEN;
    if (strstr(name, ".zip") || strstr(name, ".tar") || strstr(name, ".gz")) return COLOR_RED;
    return COLOR_RESET;
}

void print_colored(const char *name, const char *fullpath) {
    struct stat st;
    if (lstat(fullpath, &st) == -1) {
        perror("lstat failed");
        printf("%s ", name);
        return;
    }

    const char *color = get_color(name, st.st_mode);
    printf("%s%s%s", color, name, COLOR_RESET);
}

void list_directory(const char *path, int horizontal, int recursive);

void list_and_recurse(const char *path, int horizontal, int recursive) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    struct dirent *entry;
    char **filenames = NULL;
    size_t count = 0, capacity = 16;
    size_t max_len = 0;

    filenames = malloc(capacity * sizeof(char *));
    if (!filenames) {
        perror("malloc failed");
        closedir(dir);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

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

    qsort(filenames, count, sizeof(char *), compare);

    printf("\n%s:\n", path);

    int term_width = get_terminal_width();
    int col_width = max_len + SPACING;
    int cols = term_width / col_width;
    if (cols == 0) cols = 1;
    int rows = (count + cols - 1) / cols;

    if (horizontal) {
        int curr_width = 0;
        for (int i = 0; i < count; i++) {
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filenames[i]);

            if (curr_width + col_width > term_width) {
                printf("\n");
                curr_width = 0;
            }

            print_colored(filenames[i], fullpath);
            printf("%*s", col_width - (int)strlen(filenames[i]), "");
            curr_width += col_width;
        }
        printf("\n");
    } else {
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                int idx = col * rows + row;
                if (idx < count) {
                    char fullpath[1024];
                    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filenames[idx]);
                    print_colored(filenames[idx], fullpath);
                    printf("%*s", col_width - (int)strlen(filenames[idx]), "");
                }
            }
            printf("\n");
        }
    }

    // Recursively list subdirectories
    if (recursive) {
        for (int i = 0; i < count; i++) {
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filenames[i]);

            struct stat st;
            if (lstat(fullpath, &st) == -1)
                continue;

            if (S_ISDIR(st.st_mode) &&
                strcmp(filenames[i], ".") != 0 &&
                strcmp(filenames[i], "..") != 0) {
                list_directory(fullpath, horizontal, recursive);
            }
        }
    }

    for (size_t i = 0; i < count; i++)
        free(filenames[i]);
    free(filenames);
}

void list_directory(const char *path, int horizontal, int recursive) {
    list_and_recurse(path, horizontal, recursive);
}

int main(int argc, char *argv[]) {
    int opt;
    int horizontal = 0;
    int recursive = 0;
    const char *target_dir = ".";

    while ((opt = getopt(argc, argv, "xR")) != -1) {
        switch (opt) {
            case 'x': horizontal = 1; break;
            case 'R': recursive = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-x] [-R] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc)
        target_dir = argv[optind];

    list_directory(target_dir, horizontal, recursive);
    return 0;
}

