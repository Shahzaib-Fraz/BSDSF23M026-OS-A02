// src/ls.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MAX_FILENAME_LEN 256

// Display modes
typedef enum {
    DISPLAY_DEFAULT,
    DISPLAY_LONG,
    DISPLAY_HORIZONTAL
} display_mode_t;

display_mode_t display_mode = DISPLAY_DEFAULT;

// Globals for terminal width
int terminal_width = 80;

// Function declarations
void print_long_listing(char **files, int count);
void print_columns_vertical(char **files, int count);
void print_columns_horizontal(char **files, int count);
void do_ls(const char *path);
int cmpstr(const void *a, const void *b);

int main(int argc, char *argv[]) {
    // Parse args
    int opt;
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                display_mode = DISPLAY_LONG;
                break;
            case 'x':
                display_mode = DISPLAY_HORIZONTAL;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    const char *path = ".";
    if (optind < argc) {
        path = argv[optind];
    }

    // Get terminal width
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        terminal_width = w.ws_col;
    }

    do_ls(path);
    return 0;
}

// Comparison function for qsort
int cmpstr(const void *a, const void *b) {
    const char * const *pa = (const char * const *)a;
    const char * const *pb = (const char * const *)b;
    return strcmp(*pa, *pb);
}

void do_ls(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    char **files = NULL;
    int capacity = 10;
    int count = 0;

    files = malloc(capacity * sizeof(char *));
    if (!files) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files (starting with '.') for now
        if (entry->d_name[0] == '.') continue;

        if (count == capacity) {
            capacity *= 2;
            char **tmp = realloc(files, capacity * sizeof(char *));
            if (!tmp) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
            files = tmp;
        }
        files[count] = strdup(entry->d_name);
        if (!files[count]) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
        count++;
    }
    closedir(dir);

    // Sort filenames alphabetically
    qsort(files, count, sizeof(char *), cmpstr);

    // Call display functions based on display_mode
    switch (display_mode) {
        case DISPLAY_LONG:
            print_long_listing(files, count);
            break;
        case DISPLAY_HORIZONTAL:
            print_columns_horizontal(files, count);
            break;
        default:
            print_columns_vertical(files, count);
            break;
    }

    // Free memory
    for (int i = 0; i < count; i++) {
        free(files[i]);
    }
    free(files);
}

void print_long_listing(char **files, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s\n", files[i]);
    }
}

void print_columns_vertical(char **files, int count) {
    if (count == 0) return;

    // Find max filename length
    int maxlen = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(files[i]);
        if (len > maxlen) maxlen = len;
    }
    maxlen += 2; // padding

    int cols = terminal_width / maxlen;
    if (cols == 0) cols = 1;

    int rows = (count + cols - 1) / cols;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int idx = col * rows + row;
            if (idx < count) {
                printf("%-*s", maxlen, files[idx]);
            }
        }
        printf("\n");
    }
}

void print_columns_horizontal(char **files, int count) {
    if (count == 0) return;

    int maxlen = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(files[i]);
        if (len > maxlen) maxlen = len;
    }
    maxlen += 2; // padding

    int pos = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(files[i]);
        if (pos + maxlen > terminal_width) {
            printf("\n");
            pos = 0;
        }
        printf("%-*s", maxlen, files[i]);
        pos += maxlen;
    }
    printf("\n");
}
