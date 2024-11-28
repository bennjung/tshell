#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "commands.h"

int main() {
    char buf[256];
    char *argv[50];

    while (1) {
        printf("shell> ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;
        }
        buf[strcspn(buf, "\n")] = '\0';

        int narg = getargs(buf, argv);
        if (narg == 0) continue;

        if (strcmp(argv[0], "cp") == 0 && narg == 3) {
            copy_file(argv[1], argv[2]);
        } else if (strcmp(argv[0], "rm") == 0 && narg == 2) {
            delete_file(argv[1]);
        } else if (strcmp(argv[0], "mv") == 0 && narg == 3) {
            move_file(argv[1], argv[2]);
        } else if (strcmp(argv[0], "cat") == 0 && narg == 2) {
            cat_file(argv[1]);
        } else if (strcmp(argv[0], "mkdir") == 0 && narg == 2) {
            create_directory(argv[1]);
        } else if (strcmp(argv[0], "rmdir") == 0 && narg == 2) {
            remove_directory(argv[1]);
        } else if (strcmp(argv[0], "exit") == 0) {
            break;
        } else {
            printf("Unknown command: %s\n", argv[0]);
        }
    }

    return 0;
}

