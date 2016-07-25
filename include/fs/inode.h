#ifndef __FS_INODE_H_
#define __FS_INODE_H_
#include<io.h>

struct __attribute((packed)) inode_descriptor
{
	offset_t pointer;
	uint8_t typ;
	uint8_t size;
	uint32_t user,
		 group;
	uint16_t rights;
};

struct __attribute((packed)) dummy_inode
{
	offset_t pointer;
	uint64_t size;
	uint8_t name_size;
	
};

struct __attribute((packed)) link
{
	offset_t pointer;
	uint8_t name_size;
	char * name;
};

struct __attribute((packed)) directory
{
	offset_t pointer;
	uint64_t size;
	uint8_t name_size;
	char * name;
};
struct __attribute((packed)) file
{
	offset_t pointer;
	uint64_t size;
	uint8_t name_size;
	char * name;
};


/* 
   generate a buffer that can be written to the harddisk
 */
size_t directory_to_buffer(struct directory * inode, fs_word ** unallocated_buffer);
size_t link_to_buffer(struct link * inode, fs_word ** unallocated_buffer);
size_t file_to_buffer(struct file * inode, fs_word ** unallocated_buffer);
/*
 read inodes from a buffer.
 */
size_t buffer_to_file(fs_word * buffer, struct file ** inode);
size_t buffer_to_directory(fs_word * buffer, struct directory ** inode);
size_t buffer_to_link(fs_word * buffer, struct link ** inode);
void get_dummy_inode(struct inode_descriptor * inode_descriptor, struct dummy_inode ** unallocated_buffer);
#endif
