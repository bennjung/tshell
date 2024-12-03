#define DIRECTORY_OPS_H

void create_directory(char *dir_name);                // mkdir 명령어
void remove_directory(char *dir_name);                // rmdir 명령어
void create_hard_link(char *target, char *link_name); // ln 명령어 (하드 링크)
void create_symbolic_link(char *target, char *link_name); // ln -s 명령어 (심볼릭 링크)


