#ifndef __UNBUFFERED_FILE_H
#define __UNBUFFERED_FILE_H
char touch_file(struct sjfs_table * table, struct directory * parent, char * name, size_t size, uint32_t user, uint32_t group, uint16_t rights);

#endif
