#ifndef __FS_BUILD_H_
#define __FS_BUILD_H_


void sjfs_build_table(offset_t offset,
		uint64_t uuid_low, uint64_t uuid_high,
		offset_t journal_size,
		offset_t inode_descriptor_table_size,
		offset_t inode_table_size,
		offset_t size);

#endif
