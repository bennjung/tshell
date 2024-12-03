CC = gcc
CFLAGS = -Iinclude -Wall
SRCS = src/main.c src/shell_ops.c src/directory_ops.c src/file_ops.c
OBJS = $(SRCS:.c=.o)
TARGET = shell

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

