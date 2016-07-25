#ifndef __UNBUFFERED_DIRECTORY_H_
#define __UNBUFFERED_DIRECTORY_H_

#include<fs/inode.h>
void mkroot(struct sjfs_table * table);
char directory_add_child(struct sjfs_table * table, struct directory * me, offset_t child);
void mkdir(struct sjfs_table * table, struct directory * parent, char * name, uint32_t user, uint32_t group, uint16_t rights);
struct directory * get_root(struct sjfs_table * table);

#endif
