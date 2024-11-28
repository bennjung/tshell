#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int getargs(char *cmd, char **argv) {
    int narg = 0;
    while (*cmd) {
        if (*cmd == ' ' || *cmd == '\t') {
            *cmd++ = '\0';
        } else {
            argv[narg++] = cmd;
            while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t') {
                cmd++;
            }
        }
    }
    argv[narg] = NULL;
    return narg;
}

void execute_pipe(char *cmd) {
    char *commands[2];
    char *argv1[50], *argv2[50];
    int pipefd[2], pid1, pid2;

    // 파이프 분리
    commands[0] = strtok(cmd, "|");
    commands[1] = strtok(NULL, "|");

    if (commands[1] == NULL) {
        fprintf(stderr, "Error: Invalid pipe syntax\n");
        return;
    }

    // 파이프 생성
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    // 첫 번째 명령 실행
    if ((pid1 = fork()) == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        getargs(commands[0], argv1);
        execvp(argv1[0], argv1);
        perror("exec failed");
        exit(1);
    }

    // 두 번째 명령 실행
    if ((pid2 = fork()) == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        getargs(commands[1], argv2);
        execvp(argv2[0], argv2);
        perror("exec failed");
        exit(1);
    }

    // 부모 프로세스
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

int main() {
    char buf[256];
    char *argv[50];
    pid_t pid;

    while (1) {
        printf("shell> ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break; // EOF 처리
        }
        buf[strcspn(buf, "\n")] = '\0'; // 개행 문자 제거

        if (strchr(buf, '|') != NULL) {
            // 파이프 명령 처리
            execute_pipe(buf);
            continue;
        }

        int narg = getargs(buf, argv);
        if (narg == 0) {
            continue; // 빈 명령어 무시
        }

        if (strcmp(argv[0], "exit") == 0) {
            break; // "exit" 명령 처리
        }

        pid = fork();
        if (pid == 0) {
            execvp(argv[0], argv);
            perror("Command execution failed");
            exit(1);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("fork failed");
        }
    }
    return 0;
}
