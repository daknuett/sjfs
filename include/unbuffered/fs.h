#ifndef __UNBUFFERED_FS_H_
#define __UNBUFFERED_FS_H_


#include<fs/table.h>


struct sjfs_table * load_table(offset_t offset);
offset_t get_free_inode_table_space(struct sjfs_table * table, size_t requested_size);

struct inode_descriptor * get_file_close_to(struct sjfs_table * table, struct inode_descriptor * inode_descriptor);
offset_t get_free_file_space(struct sjfs_table * table, size_t requested_size);
/* returns 0 on failure */
offset_t get_inode_descriptor_offset_by_name(struct sjfs_table * table, char * name);
#endif
