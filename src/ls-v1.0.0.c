#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

void list_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files
        if (entry->d_name[0] != '.')
            printf("%s  ", entry->d_name);
    }

    printf("\n");
    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *target_dir = ".";  // Default directory

    if (argc > 1) {
        target_dir = argv[1];
    }

    list_directory(target_dir);
    return 0;
}

