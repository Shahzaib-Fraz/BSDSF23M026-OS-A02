#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// ANSI color codes
#define COLOR_RESET     "\033[0m"
#define COLOR_BLUE      "\033[0;34m"
#define COLOR_GREEN     "\033[0;32m"
#define COLOR_RED       "\033[0;31m"
#define COLOR_PINK      "\033[0;35m"
#define COLOR_REVERSE   "\033[7m"

// Display modes
typedef enum {
    DISPLAY_DEFAULT,
    DISPLAY_LONG,
    DISPLAY_HORIZONTAL
} display_mode_t;

display_mode_t display_mode = DISPLAY_DEFAULT;

// Helper to check executable bit for user, group, or others
int is_executable(mode_t mode) {
    return (mode & S_IXUSR) || (mode & S_IXGRP) || (mode & S_IXOTH);
}

// Helper: check if file is archive (ends with .tar, .gz, or .zip)
int is_archive(const char *name) {
    const char *ext = strrchr(name, '.');
    if (!ext) return 0;
    return (strcmp(ext, ".tar") == 0 || strcmp(ext, ".gz") == 0 || strcmp(ext, ".zip") == 0);
}

// Print filename with color based on file type
void print_colored(const char *filename, const char *fullpath) {
    struct stat st;
    int stat_res;

    // Use lstat to identify symlinks
    stat_res = lstat(fullpath, &st);
    if (stat_res < 0) {
        perror("lstat");
        printf("%s ", filename);
        return;
    }

    if (S_ISLNK(st.st_mode)) {
        printf(COLOR_PINK "%s" COLOR_RESET " ", filename);
    } else if (S_ISDIR(st.st_mode)) {
        printf(COLOR_BLUE "%s" COLOR_RESET " ", filename);
    } else if (is_executable(st.st_mode)) {
        printf(COLOR_GREEN "%s" COLOR_RESET " ", filename);
    } else if (is_archive(filename)) {
        printf(COLOR_RED "%s" COLOR_RESET " ", filename);
    } else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode)) {
        // Special files: reverse video
        printf(COLOR_REVERSE "%s" COLOR_RESET " ", filename);
    } else {
        // Normal file, no color
        printf("%s ", filename);
    }
}

// For simplicity, implement just horizontal output for demonstration
void print_horizontal(char **names, int count) {
    int i;
    for (i = 0; i < count; i++) {
        // Build full path for stat
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "./%s", names[i]);
        print_colored(names[i], fullpath);
    }
    printf("\n");
}

int compare_names(const void *a, const void *b) {
    const char **pa = (const char **)a;
    const char **pb = (const char **)b;
    return strcmp(*pa, *pb);
}

int main(int argc, char *argv[]) {
    int opt;
    char *dirname = ".";
    display_mode = DISPLAY_DEFAULT;

    while ((opt = getopt(argc, argv, "xl")) != -1) {
        switch (opt) {
            case 'x':
                display_mode = DISPLAY_HORIZONTAL;
                break;
            case 'l':
                display_mode = DISPLAY_LONG;
                break;
            default:
                fprintf(stderr, "Usage: %s [-x] [-l] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        dirname = argv[optind];
    }

    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    char **names = NULL;
    size_t capacity = 0;
    size_t count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files (starting with .)
        if (entry->d_name[0] == '.') continue;

        if (count == capacity) {
            capacity = capacity == 0 ? 64 : capacity * 2;
            names = realloc(names, capacity * sizeof(char *));
            if (!names) {
                perror("realloc");
                closedir(dir);
                exit(EXIT_FAILURE);
            }
        }
        names[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    // Sort names alphabetically
    qsort(names, count, sizeof(char *), compare_names);

    // Print based on display mode
    switch (display_mode) {
        case DISPLAY_HORIZONTAL:
            print_horizontal(names, count);
            break;
        case DISPLAY_LONG:
            // For demo, just print names with newline in long mode (no color here)
            for (size_t i = 0; i < count; i++) {
                printf("%s\n", names[i]);
            }
            break;
        default:
            // Default is horizontal but without colors for demo
            print_horizontal(names, count);
            break;
    }

    // Free allocated memory
    for (size_t i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);

    return 0;
}
