#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include "shell_ops.h"

// ls 명령어
void execute_ls() {
    DIR *dir = opendir(".");
    struct dirent *entry;

    if (dir == NULL) {
        perror("opendir failed");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            printf("%s\n", entry->d_name);
        }
    }
    closedir(dir);
}

// pwd 명령어
void execute_pwd() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return;
    }
    printf("%s\n", cwd);
}

// cd 명령어
void execute_cd(char *path) {
    if (chdir(path) == -1) {
        perror("chdir failed");
    }
}

