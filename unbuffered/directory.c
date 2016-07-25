#include<io.h>
#include<fs/inode.h>
#include<unbuffered/fs.h>
#include<journal/journal.h>
#include<string.h>

void mkroot(struct sjfs_table * table)
{
	struct directory * root = calloc(sizeof(struct directory), 1);
	struct inode_descriptor * root_descr = calloc(sizeof(struct inode_descriptor), 1);


	root->pointer = table->file_space_pointer;
	// this is ./ and ../ (both root)
	root->size = 2 * sizeof(offset_t);
	root->name_size = 0;
	root->name = "";

	offset_t * this =  &table->inode_table_pointer;
	write_buffer(table->file_space_pointer, sizeof(offset_t), (fs_word **) &this);
	write_buffer(table->file_space_pointer + sizeof(offset_t), sizeof(offset_t),(fs_word **) &this);

	root_descr->pointer = table->inode_table_pointer;
	root_descr->typ = 'D';
	root_descr->size = 18;
	root_descr->user = 1;
	root_descr->group = 1;
	root_descr->rights = 0777; // rwxrwxrwx

	char null = 0;
	fs_word * buffer = NULL;
	size_t to_write = directory_to_buffer(root, &buffer);
	write_buffer(table->inode_table_pointer, to_write, &buffer);
	free(buffer);


	write_buffer(table->inode_descriptor_table_pointer,
			sizeof(struct inode_descriptor),
			(fs_word **) &root_descr);
}

struct directory * get_root(struct sjfs_table * table)
{
	struct directory * root;
	struct inode_descriptor * root_descr = calloc(sizeof(struct inode_descriptor), 1);

	read_allocated_buffer(table->inode_descriptor_table_pointer, 
			sizeof(struct inode_descriptor),
			(fs_word **) &root_descr);
	fs_word * buffer = malloc(root_descr->size);
	read_allocated_buffer(root_descr->pointer, root_descr->size, &buffer);
	buffer_to_directory(buffer, &root);
	free(buffer);
	free(root_descr);
	return root;
}

char directory_add_child(struct sjfs_table * table, struct directory * me, offset_t child)
{
	struct journal_entry * buffered_me = buffer_directory(table, me, sizeof(offset_t), MODE_A);
	if(!buffered_me)
	{
		return -1;
	}
	offset_t * child_p = &child;
	write_buffer(buffered_me->dbuffer_cursor, sizeof(offset_t),(fs_word **) &child_p);
	buffered_me->dbuffer_cursor += sizeof(offset_t);
	buffered_me->dbuffer_eof += sizeof(offset_t);
	return unbuffer_entry(table, buffered_me);

}

char mkdir(struct sjfs_table * table, struct directory * parent, char * name, uint32_t user, uint32_t group, uint16_t rights)
{
	struct directory * dir = calloc(sizeof(struct directory), 1);
	dir->name = name;
	struct inode_descriptor * dir_descr = calloc(sizeof(struct inode_descriptor), 1);

	// this will be copied later to the disk
	fs_word * buffer = NULL;

	// XXX get free file space
	// get the size of the file
	size_t size = sizeof(offset_t) * 2;
	offset_t space = get_free_file_space(table, size);
	if(space == 0)
	{
		return -1;
	}

	dir->pointer = space;
	dir->name_size = strlen(name);
	dir->size = size;

	// size of the inode:
	size = directory_to_buffer(dir, &buffer);
	offset_t it_pointer = get_free_inode_table_space(table, size);
	dir_descr->pointer = it_pointer;
	dir_descr->size = size;
	dir_descr->user = user;
	dir_descr->group = group;
	dir_descr->rights = rights;
	dir_descr->typ = 'D';

	// write the directory:
	write_buffer(it_pointer, size, &buffer);
	free(buffer);

	// write the initial child list
	// the hard way :-P
	offset_t * ptr = &it_pointer;
	write_buffer(space, sizeof(offset_t), (fs_word **) &ptr); 
	space += sizeof(offset_t);

	// we need the parent directory's address
	offset_t parent_addr = get_inode_descriptor_offset_by_name(table, parent->name);
	ptr = &parent_addr;
	write_buffer(space, sizeof(offset_t), (fs_word **) &ptr); 


	// write the inode_descriptor
	table->last_inode_descriptor += sizeof(struct inode_descriptor);
	write_buffer(table->last_inode_descriptor, sizeof(struct inode_descriptor), (fs_word **) &dir_descr);

	// hey! I am new here!

	directory_add_child(table, parent, table->last_inode_descriptor );
	return 0;
}

