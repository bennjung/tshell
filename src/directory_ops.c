#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "directory_ops.h"

void create_directory(char *dir_name) {
    if (mkdir(dir_name, 0755) == -1) {
        perror("mkdir");
        return;
    }
    printf("Directory '%s' created successfully.\n", dir_name);
}

void remove_directory(char *dir_name) {
    if (rmdir(dir_name) == -1) {
        perror("rmdir");
        return;
    }
    printf("Directory '%s' removed successfully.\n", dir_name);
}

