#define FILE_OPS_H

void copy_file(const char *source, const char *destination);
void delete_file(const char *file);
void move_file(const char *source, const char *destination);
void cat_file(const char *file);
