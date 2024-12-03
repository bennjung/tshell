#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_ARGS 50
#define MAX_BUF 256

// 입력을 공백 기준으로 토큰화
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

// 파이프 명령어 처리
void execute_pipe(char *cmd) {
    char *commands[2];
    char *argv1[MAX_ARGS], *argv2[MAX_ARGS];
    int pipefd[2], pid1, pid2;

    commands[0] = strtok(cmd, "|");
    commands[1] = strtok(NULL, "|");

    if (commands[1] == NULL) {
        fprintf(stderr, "Error: Invalid pipe syntax\n");
        return;
    }

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
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

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// SIGINT와 SIGQUIT 처리
void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nSIGINT (Ctrl-C) received. Use 'exit' to quit.\n");
    } else if (signo == SIGQUIT) {
        printf("\nSIGQUIT (Ctrl-\\) received. Use 'exit' to quit.\n");
    }
}

int main() {
    char buf[MAX_BUF];
    char *argv[MAX_ARGS];
    pid_t pid;

    // SIGINT와 SIGQUIT 신호 처리
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("Unable to set SIGINT handler");
    }
    if (signal(SIGQUIT, sig_handler) == SIG_ERR) {
        perror("Unable to set SIGQUIT handler");
    }

    while (1) {
        printf("shell> ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;
        }
        buf[strcspn(buf, "\n")] = '\0';

        // 백그라운드 실행 여부 확인
        int background = 0;
        if (buf[strlen(buf) - 1] == '&') {
            background = 1;
            buf[strlen(buf) - 1] = '\0'; // '&' 제거
        }

        // 파일 재지향 처리
        int redir_in = -1, redir_out = -1;
        char *redir_in_pos = strchr(buf, '<');
        char *redir_out_pos = strchr(buf, '>');
        if (redir_in_pos) {
            *redir_in_pos = '\0'; // 입력 명령 분리
            char *infile = strtok(redir_in_pos + 1, " \t");
            redir_in = open(infile, O_RDONLY);
            if (redir_in == -1) {
                perror("Error opening input file");
                continue;
            }
        }
        if (redir_out_pos) {
            *redir_out_pos = '\0'; // 출력 명령 분리
            char *outfile = strtok(redir_out_pos + 1, " \t");
            redir_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (redir_out == -1) {
                perror("Error opening output file");
                continue;
            }
        }

        // 파이프 처리
        if (strchr(buf, '|') != NULL) {
            execute_pipe(buf);
            continue;
        }

        // 명령어 파싱
        int narg = getargs(buf, argv);
        if (narg == 0) {
            continue;
        }

        // "exit" 명령 처리
        if (strcmp(argv[0], "exit") == 0) {
            break;
        }

        // 자식 프로세스 생성
        pid = fork();
        if (pid == 0) {
            // 파일 재지향 처리
            if (redir_in != -1) {
                dup2(redir_in, STDIN_FILENO);
                close(redir_in);
            }
            if (redir_out != -1) {
                dup2(redir_out, STDOUT_FILENO);
                close(redir_out);
            }
            execvp(argv[0], argv);
            perror("Command execution failed");
            exit(1);
        } else if (pid > 0) {
            if (!background) {
                waitpid(pid, NULL, 0);
            }
        } else {
            perror("fork failed");
        }

        if (redir_in != -1) close(redir_in);
        if (redir_out != -1) close(redir_out);
    }

    return 0;
}

