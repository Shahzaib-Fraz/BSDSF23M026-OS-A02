#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// ANSI color codes from previous version
#define COLOR_RESET     "\033[0m"
#define COLOR_BLUE      "\033[0;34m"
#define COLOR_GREEN     "\033[0;32m"
#define COLOR_RED       "\033[0;31m"
#define COLOR_PINK      "\033[0;35m"
#define COLOR_REVERSE   "\033[7m"

int recursive_flag = 0;

int is_executable(mode_t mode) {
    return (mode & S_IXUSR) || (mode & S_IXGRP) || (mode & S_IXOTH);
}

int is_archive(const char *name) {
    const char *ext = strrchr(name, '.');
    if (!ext) return 0;
    return (strcmp(ext, ".tar") == 0 || strcmp(ext, ".gz") == 0 || strcmp(ext, ".zip") == 0);
}

void print_colored(const char *filename, const char *fullpath) {
    struct stat st;
    if (lstat(fullpath, &st) < 0) {
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
        printf(COLOR_REVERSE "%s" COLOR_RESET " ", filename);
    } else {
        printf("%s ", filename);
    }
}

int compare_names(const void *a, const void *b) {
    const char **pa = (const char **)a;
    const char **pb = (const char **)b;
    return strcmp(*pa, *pb);
}

void do_ls(const char *dirname);

void list_directory(const char *dirname) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror(dirname);
        return;
    }

    struct dirent *entry;
    char **names = NULL;
    size_t capacity = 0;
    size_t count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // skip hidden files (optional, but standard ls only shows them with -a)
        if (entry->d_name[0] == '.') continue;

        if (count == capacity) {
            capacity = capacity == 0 ? 64 : capacity * 2;
            names = realloc(names, capacity * sizeof(char *));
            if (!names) {
                perror("realloc");
                closedir(dir);
                return;
            }
        }
        names[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    qsort(names, count, sizeof(char *), compare_names);

    for (size_t i = 0; i < count; i++) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, names[i]);
        print_colored(names[i], fullpath);
    }
    printf("\n");

    if (recursive_flag) {
        for (size_t i = 0; i < count; i++) {
            if (strcmp(names[i], ".") == 0 || strcmp(names[i], "..") == 0) {
                free(names[i]);
                continue;
            }
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, names[i]);

            struct stat st;
            if (lstat(fullpath, &st) == -1) {
                perror("lstat");
                free(names[i]);
                continue;
            }

            if (S_ISDIR(st.st_mode)) {
                printf("\n%s:\n", fullpath);
                do_ls(fullpath);  // recursive call
            }
            free(names[i]);
        }
    } else {
        for (size_t i = 0; i < count; i++) {
            free(names[i]);
        }
    }
    free(names);
}

void do_ls(const char *dirname) {
    list_directory(dirname);
}

int main(int argc, char *argv[]) {
    int opt;
    char *dirname = ".";
    recursive_flag = 0;

    while ((opt = getopt(argc, argv, "xRl")) != -1) {
        switch (opt) {
            case 'x':
                // For simplicity, ignoring different display modes here
                break;
            case 'l':
                // ignoring long-listing for now
                break;
            case 'R':
                recursive_flag = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-x] [-l] [-R] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        dirname = argv[optind];
    }

    if (recursive_flag) {
        printf("%s:\n", dirname);
    }

    do_ls(dirname);

    return 0;
}
