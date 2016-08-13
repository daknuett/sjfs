#ifndef __UNBUFFERED_FILE_H
#define __UNBUFFERED_FILE_H
char touch_file(struct sjfs_table * table, struct directory * parent, char * name, size_t size, uint32_t user, uint32_t group, uint16_t rights);
struct journal_entry * open_file(struct sjfs_table * table, struct file * file, char mode);
int __file_putc(struct journal_entry * entry, char __c);

#endif
