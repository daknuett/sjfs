#include<io.h>
#include<fs/inode.h>
#include<unbuffered/fs.h>
#include<journal/journal.h>
#include<unbuffered/directory.h>
#include<string.h>


char touch_file(struct sjfs_table * table, struct directory * parent, char * name, size_t size, uint32_t user, uint32_t group, uint16_t rights)
{
	struct file * file = calloc(sizeof(struct file), 1);
	file->name = name;
	fs_word * buffer;
	offset_t space = get_free_file_space(table, size);
	if(space == 0)
	{
		return -1;
	}
	file->pointer = space;
	file->name_size = strlen(name);
	file->size = size;

	// now inode desciptor
	struct inode_descriptor * file_descr = calloc(sizeof(struct inode_descriptor), 1);
	size = file_to_buffer(file, &buffer);
	file_descr->size = size;
	offset_t it_pointer = get_free_inode_table_space(table, size);
	file_descr->pointer = it_pointer;
	file_descr->typ = 'F';
	file_descr->user = user;
	file_descr->group = group;
	file_descr->rights = rights;
	write_buffer(it_pointer, size, &buffer);
	free(buffer);

	// write the inode_descriptor
	table->last_inode_descriptor += sizeof(struct inode_descriptor);
	write_buffer(table->last_inode_descriptor, sizeof(struct inode_descriptor), (fs_word **) &file_descr);
	directory_add_child(table, parent, table->last_inode_descriptor );
	return 0;
}
