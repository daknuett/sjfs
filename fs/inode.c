#include<io.h>
#include<stdlib.h>
#include<string.h>

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
	uint64_t padding;
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

size_t link_to_buffer(struct link * inode, fs_word ** unallocated_buffer)
{
	size_t pointer = sizeof(struct link) - sizeof(char * );
	fs_word * buffer = malloc(pointer + strlen(inode->name) + 1);
	memcpy(buffer, inode, pointer);
	memcpy(buffer + pointer, inode->name, strlen(inode->name));
	*unallocated_buffer = buffer;
	return pointer + strlen(inode->name) + 1;
}

size_t directory_to_buffer(struct directory * inode, fs_word ** unallocated_buffer)
{
	size_t pointer = sizeof(struct directory) - sizeof(char * );
	fs_word * buffer = malloc(pointer + strlen(inode->name) + 1);
	memcpy(buffer, inode, pointer);
	memcpy(buffer + pointer, inode->name, strlen(inode->name));
	*unallocated_buffer = buffer;
	return pointer + strlen(inode->name) + 1;
}

size_t file_to_buffer(struct file * inode, fs_word ** unallocated_buffer)
{
	size_t pointer = sizeof(struct file) - sizeof(char * );
	fs_word * buffer = malloc(pointer + strlen(inode->name) + 1);
	memcpy(buffer, inode, pointer);
	memcpy(buffer + pointer, inode->name, strlen(inode->name));
	*unallocated_buffer = buffer;
	return pointer + strlen(inode->name) + 1;
}


size_t buffer_to_link(fs_word * buffer, struct link ** inode)
{
	struct link * raw = (struct link *)buffer;
	// alloc space for the name
	char * name = malloc(raw->name_size + 1);
	// alloc space for the link
	struct link * link = malloc(sizeof(struct link));

	size_t pointer = sizeof(struct link) - sizeof(char * );
	// copy the packed data
	memcpy(link, buffer, pointer);
	// now copy the string
	memcpy(name, buffer + pointer, raw->name_size);
	link->name = name;
	// done
	*inode = link;
	return pointer + raw->name_size;
}
size_t buffer_to_directory(fs_word * buffer, struct directory ** inode)
{
	struct directory * raw = (struct directory * )buffer;
	// alloc space for the name
	char * name = malloc(raw->name_size + 1);
	// alloc space for the directory
	struct directory * directory = malloc(sizeof(struct directory));

	size_t pointer = sizeof(struct directory) - sizeof(char * );
	// copy the packed data
	memcpy(directory, buffer, pointer);
	// now copy the string
	memcpy(name, buffer + pointer, raw->name_size);
	directory->name = name;
	// done
	*inode = directory;
	return pointer + raw->name_size;
}
size_t buffer_to_file(fs_word * buffer, struct file ** inode)
{
	struct file * raw = (struct file * )buffer;
	// alloc space for the name
	char * name = malloc(raw->name_size + 1);
	// alloc space for the file
	struct file * file = malloc(sizeof(struct file));

	size_t pointer = sizeof(struct file) - sizeof(char * );
	// copy the packed data
	memcpy(file, buffer, pointer);
	// now copy the string
	memcpy(name, buffer + pointer, raw->name_size);
	file->name = name;
	// done
	*inode = file;
	return pointer + raw->name_size;
}

void get_dummy_inode(struct inode_descriptor * inode_descriptor, struct dummy_inode ** unallocated_buffer)
{
	struct dummy_inode * inode = calloc(sizeof(struct dummy_inode), 1);
	read_allocated_buffer(inode_descriptor->pointer, sizeof(struct dummy_inode), (fs_word **) &inode);
	*unallocated_buffer = inode;
}
