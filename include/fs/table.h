#include<stdint.h>
#ifndef __FS_TABLE_H_
#define __FS_TABLE_H_

struct __attribute((packed)) sjfs_table 
{
	uint64_t uuid_low, uuid_high;
	uint64_t size;
	uint8_t typ;
	uint64_t journal_pointer,
		 journal_size,
		 inode_descriptor_table_pointer,
		 inode_descriptor_table_size,
		 last_inode_descriptor,
		 inode_table_pointer,
		 inode_table_size,
		 file_space_pointer;
};

#endif
