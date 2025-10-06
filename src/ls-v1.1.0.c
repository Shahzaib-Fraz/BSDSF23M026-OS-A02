#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#include <errno.h>

void print_permissions(mode_t mode) {
    char perms[11] = "----------";

    // File type
    if (S_ISDIR(mode)) perms[0] = 'd';
    else if (S_ISLNK(mode)) perms[0] = 'l';
    else if (S_ISCHR(mode)) perms[0] = 'c';
    else if (S_ISBLK(mode)) perms[0] = 'b';
    else if (S_ISFIFO(mode)) perms[0] = 'p';
    else if (S_ISSOCK(mode)) perms[0] = 's';

    // Owner
    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = 'x';

    // Group
    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = 'x';

    // Others
    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = 'x';

    printf("%s ", perms);
}

void list_long_format(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files
        if (entry->d_name[0] == '.')
            continue;

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat sb;
        if (lstat(fullpath, &sb) == -1) {
            perror("lstat");
            continue;
        }

        // Permissions
        print_permissions(sb.st_mode);

        // Links
        printf("%2ld ", (long)sb.st_nlink);

        // User and Group
        struct passwd *pw = getpwuid(sb.st_uid);
        struct group *gr = getgrgid(sb.st_gid);

        printf("%s %s ", 
               pw ? pw->pw_name : "UNKNOWN", 
               gr ? gr->gr_name : "UNKNOWN");

        // Size
        printf("%6ld ", (long)sb.st_size);

        // Modification time
        char *time_str = ctime(&sb.st_mtime);
        time_str[strlen(time_str) - 1] = '\0'; // Remove newline
        printf("%s ", time_str);

        // File name
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

void list_simple(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files
        if (entry->d_name[0] == '.')
            continue;

        printf("%s  ", entry->d_name);
    }

    printf("\n");
    closedir(dir);
}

int main(int argc, char *argv[]) {
    int opt;
    int long_listing = 0;

    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l':
                long_listing = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Directory path: if given as arg, else default to "."
    const char *path = (optind < argc) ? argv[optind] : ".";

    if (long_listing)
        list_long_format(path);
    else
        list_simple(path);

    return 0;
}
