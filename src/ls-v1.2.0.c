#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#define SPACING 2  // Spaces between columns

int compare(const void *a, const void *b) {
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb);
}

int get_terminal_width() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        return 80;  // Fallback width
    }
    return ws.ws_col;
}

void list_column_display(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    // Read all entries
    char **filenames = NULL;
    size_t count = 0, capacity = 16;
    filenames = malloc(capacity * sizeof(char *));
    if (!filenames) {
        perror("malloc failed");
        closedir(dir);
        return;
    }

    struct dirent *entry;
    size_t max_len = 0;

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

    // Sort filenames alphabetically
    qsort(filenames, count, sizeof(char *), compare);

    // Calculate columns and rows
    int term_width = get_terminal_width();
    int col_width = max_len + SPACING;
    int cols = term_width / col_width;
    if (cols == 0) cols = 1;
    int rows = (count + cols - 1) / cols;

    // Print down then across
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int idx = col * rows + row;
            if (idx < count) {
                printf("%-*s", col_width, filenames[idx]);
            }
        }
        printf("\n");
    }

    for (size_t i = 0; i < count; i++) {
        free(filenames[i]);
    }
    free(filenames);
}

int main(int argc, char *argv[]) {
    int opt;
    const char *target_dir = ".";

    while ((opt = getopt(argc, argv, "")) != -1) {
        // No options yet
    }

    if (optind < argc) {
        target_dir = argv[optind];
    }

    list_column_display(target_dir);
    return 0;
}

