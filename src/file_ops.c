#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "file_ops.h"

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

void delete_file(const char *file) {
    if (unlink(file) == -1) {
        perror("File delete failed");
        return;
    }
    printf("File '%s' deleted successfully\n", file);
}

void move_file(const char *source, const char *destination) {
    if (rename(source, destination) == -1) {
        perror("File move failed");
        return;
    }
    printf("File '%s' moved to '%s'\n", source, destination);
}

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

