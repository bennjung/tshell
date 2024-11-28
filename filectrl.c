#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

// 디렉터리 생성 함수
void create_directory(char *dir_name) {
    if (mkdir(dir_name, 0755) == -1) {
        perror("mkdir");
        return;
    }
    printf("Directory '%s' created successfully.\n", dir_name);
}

// 디렉터리 삭제 함수
void remove_directory(char *dir_name) {
    if (rmdir(dir_name) == -1) {
        perror("rmdir");
        return;
    }
    printf("Directory '%s' removed successfully.\n", dir_name);
}

// 파일 복사 함수
void copy_file(const char *source, const char *destination) {
    int src_fd, dest_fd;
    char buffer[4096];
    ssize_t bytes_read, bytes_written;

    src_fd = open(source, O_RDONLY);
    if (src_fd == -1) {
        perror("Source file open failed");
        return;
    }

    dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        perror("Destination file open failed");
        close(src_fd);
        return;
    }

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("File write failed");
            close(src_fd);
            close(dest_fd);
            return;
        }
    }

    if (bytes_read == -1) {
        perror("File read failed");
    }

    close(src_fd);
    close(dest_fd);
    printf("File '%s' copied to '%s'\n", source, destination);
}

// 파일 삭제 함수
void delete_file(const char *file) {
    if (unlink(file) == -1) {
        perror("File delete failed");
        return;
    }
    printf("File '%s' deleted successfully\n", file);
}

// 파일 이동 함수
void move_file(const char *source, const char *destination) {
    if (rename(source, destination) == -1) {
        perror("File move failed");
        return;
    }
    printf("File '%s' moved to '%s'\n", source, destination);
}

// 파일 내용 출력 함수
void cat_file(const char *file) {
    int fd;
    char buffer[4096];
    ssize_t bytes_read;

    fd = open(file, O_RDONLY);
    if (fd == -1) {
        perror("File open failed");
        return;
    }

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    if (bytes_read == -1) {
        perror("File read failed");
    }

    close(fd);
}

// 명령어를 분리하는 함수
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

// 파이프 명령어 처리 함수
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
    char buf[256];
    char *argv[50];
    pid_t pid;

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
            break;
        }
        buf[strcspn(buf, "\n")] = '\0';

        int narg = getargs(buf, argv);
        if (narg == 0) {
            continue;
        }

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
        } else if (strchr(buf, '|') != NULL) {
            execute_pipe(buf);
        } else {
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
    }
    return 0;
}
