#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

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

void create_hard_link(char *target, char *link_name) {
    if (link(target, link_name) == -1) {
        perror("link");
        return;
    }
    printf("Hard link '%s' created for '%s'.\n", link_name, target);
}

void create_symbolic_link(char *target, char *link_name) {
    if (symlink(target, link_name) == -1) {
        perror("symlink");
        return;
    }
    printf("Symbolic link '%s' created for '%s'.\n", link_name, target);
}
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

// SIGINT와 SIGTSTP를 처리하는 핸들러 함수
void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nSIGINT received: Process interrupted.\n");
    } else if (signo == SIGTSTP) {
        printf("\nSIGTSTP received: Process stopped.\n");
    }
}

int main() {
    char buf[256];    // 사용자 입력 버퍼
    char *argv[50];   // 명령어와 인수를 저장할 배열
    pid_t pid;

    // SIGINT와 SIGTSTP에 대한 신호 핸들러 설정
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("Unable to catch SIGINT");
        exit(1);
    }
    if (signal(SIGTSTP, sig_handler) == SIG_ERR) {
        perror("Unable to catch SIGTSTP");
        exit(1);
    }

    while (1) {
        printf("shell> ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break; // EOF 처리
        }
        buf[strcspn(buf, "\n")] = '\0'; // 개행 문자 제거

        // '&'가 있으면 백그라운드 실행
        int background = 0;
        if (buf[strlen(buf) - 1] == '&') {
            background = 1;
            buf[strlen(buf) - 1] = '\0'; // '&' 제거
        }

        // 파일 재지향 처리
        char *redir_pos = strchr(buf, '>');
        if (redir_pos != NULL) {
            // '>'를 발견하면, 재지향 처리
            *redir_pos = '\0'; // 명령어와 '>' 분리
            char *outfile = strtok(redir_pos + 1, " \t"); // 출력 파일 이름

            int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror("Error opening file for output redirection");
                continue;
            }

            pid = fork();
            if (pid == 0) {
                dup2(fd, STDOUT_FILENO);  // 표준 출력을 파일로 변경
                close(fd);
                int narg = getargs(buf, argv);
                execvp(argv[0], argv); // 명령 실행
                perror("Command execution failed");
                exit(1);
            } else if (pid > 0) {
                if (!background) {
                    wait(NULL); // 부모는 자식 종료 대기
                }
            } else {
                perror("fork failed");
            }
            continue;
        }

        if (strchr(buf, '|') != NULL) {
            // 파이프 명령 처리
            execute_pipe(buf);
            continue;
        }

        int narg = getargs(buf, argv);
        if (narg == 0) {
            continue; // 빈 명령어 무시
        }

        // "mkdir"와 "rmdir" 명령어 처리
        if (strcmp(argv[0], "mkdir") == 0 && narg == 2) {
            create_directory(argv[1]);  // mkdir <directory_name>
        } else if (strcmp(argv[0], "rmdir") == 0 && narg == 2) {
            remove_directory(argv[1]);  // rmdir <directory_name>
        } else if (strcmp(argv[0], "exit") == 0) {
            break; // "exit" 명령 처리
        } else {
            pid = fork();
            if (pid == 0) {
                execvp(argv[0], argv);  // 다른 명령어 실행
                perror("Command execution failed");
                exit(1);
            } else if (pid > 0) {
                if (!background) {
                    // 백그라운드가 아니라면 부모 프로세스가 자식 종료를 기다림
                    wait(NULL);
                }
            } else {
                perror("fork failed");
            }
        }
    }
    return 0;
} 