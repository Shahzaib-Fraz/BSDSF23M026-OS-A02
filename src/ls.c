#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define MAX_FILES 1024
#define MAX_FILENAME_LEN 256

enum DisplayMode {
    DEFAULT,
    LONG_LIST,
    HORIZONTAL
};

void get_terminal_size(int *width) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *width = w.ws_col;
}

int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void print_long_list(const char *dirname, char *files[], int count) {
    struct stat fileStat;
    char path[1024];

    for (int i = 0; i < count; i++) {
        snprintf(path, sizeof(path), "%s/%s", dirname, files[i]);

        if (stat(path, &fileStat) == -1) {
            perror("stat");
            continue;
        }

        printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
        printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
        printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
        printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");

        struct passwd *pw = getpwuid(fileStat.st_uid);
        struct group  *gr = getgrgid(fileStat.st_gid);
        char timebuf[80];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&fileStat.st_mtime));

        printf(" %2lu %s %s %6ld %s %s\n",
               fileStat.st_nlink,
               pw ? pw->pw_name : "???",
               gr ? gr->gr_name : "???",
               fileStat.st_size,
               timebuf,
               files[i]);
    }
}

void print_vertical(char *files[], int count, int terminal_width) {
    int max_len = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(files[i]);
        if (len > max_len)
            max_len = len;
    }

    int col_width = max_len + 2;
    int cols = terminal_width / col_width;
    if (cols == 0) cols = 1;
    int rows = (count + cols - 1) / cols;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int idx = col * rows + row;
            if (idx < count) {
                printf("%-*s", col_width, files[idx]);
            }
        }
        printf("\n");
    }
}

void print_horizontal(char *files[], int count, int terminal_width) {
    int max_len = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(files[i]);
        if (len > max_len)
            max_len = len;
    }

    int col_width = max_len + 2;
    int cols = terminal_width / col_width;
    if (cols == 0) cols = 1;

    for (int i = 0; i < count; i++) {
        printf("%-*s", col_width, files[i]);

        if ((i + 1) % cols == 0)
            printf("\n");
    }

    if (count % cols != 0)
        printf("\n");
}

int main(int argc, char *argv[]) {
    enum DisplayMode mode = DEFAULT;
    int opt;

    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                mode = LONG_LIST;
                break;
            case 'x':
                mode = HORIZONTAL;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [dir]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    const char *dirname = ".";
    if (optind < argc)
        dirname = argv[optind];

    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    char *files[MAX_FILES];
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.')
            files[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    qsort(files, count, sizeof(char *), compare);

    int terminal_width = 80;
    get_terminal_size(&terminal_width);

    switch (mode) {
        case LONG_LIST:
            print_long_list(dirname, files, count);
            break;
        case HORIZONTAL:
            print_horizontal(files, count, terminal_width);
            break;
        default:
            print_vertical(files, count, terminal_width);
    }

    for (int i = 0; i < count; i++)
        free(files[i]);

    return 0;
}
