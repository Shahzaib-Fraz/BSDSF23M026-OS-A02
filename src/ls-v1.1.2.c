#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define INITIAL_CAPACITY 32
#define COLUMN_SPACING 2

// Get terminal width using ioctl
void get_terminal_width(int *width) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        *width = ws.ws_col;
    } else {
        *width = 80; // fallback default
    }
}

// For alphabetical sorting
int compare_strings(const void *a, const void *b) {
    const char *s1 = *(const char **)a;
    const char *s2 = *(const char **)b;
    return strcmp(s1, s2);
}

int main(void) {
    DIR *dir;
    struct dirent *entry;
    char **filenames = NULL;
    int capacity = INITIAL_CAPACITY;
    int count = 0;
    size_t max_len = 0;

    filenames = malloc(capacity * sizeof(char *));
    if (!filenames) {
        perror("malloc");
        return 1;
    }

    dir = opendir(".");
    if (!dir) {
        perror("opendir");
        return 1;
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip hidden files

        if (count == capacity) {
            capacity *= 2;
            filenames = realloc(filenames, capacity * sizeof(char *));
            if (!filenames) {
                perror("realloc");
                return 1;
            }
        }

        filenames[count] = strdup(entry->d_name);
        if (!filenames[count]) {
            perror("strdup");
            return 1;
        }

        size_t len = strlen(entry->d_name);
        if (len > max_len) max_len = len;

        count++;
    }
    closedir(dir);

    if (count == 0) {
        free(filenames);
        return 0;
    }

    // Sort entries alphabetically
    qsort(filenames, count, sizeof(char *), compare_strings);

    // Get terminal width
    int term_width;
    get_terminal_width(&term_width);

    int col_width = max_len + COLUMN_SPACING;
    int num_cols = term_width / col_width;
    if (num_cols < 1) num_cols = 1;

    int num_rows = (count + num_cols - 1) / num_cols;

    // Print in down-then-across format
    for (int row = 0; row < num_rows; row++) {
        for (int col = 0; col < num_cols; col++) {
            int index = col * num_rows + row;
            if (index < count) {
                printf("%-*s", col_width, filenames[index]);
            }
        }
        printf("\n");
    }

    // Cleanup
    for (int i = 0; i < count; i++) {
        free(filenames[i]);
    }
    free(filenames);

    return 0;
}
