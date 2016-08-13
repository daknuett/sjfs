#include<io.h>
#include<stdio.h>
#include<fs/inode.h>
#include<unbuffered/fs.h>
#include<journal/journal.h>
#include<unbuffered/directory.h>
#include<string.h>

#define STD_BUFS 30

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


struct journal_entry * open_file(struct sjfs_table * table, struct file * file, char mode)
{
	
	struct journal_entry * buffered;
	if(mode & WEN)
	{
		buffered = buffer_file(table, file, STD_BUFS, mode);
		if(!buffered)
		{
			return NULL;
		}
	}
	else
	{
		// manually setup a pseudo entry
		// the file will not be copied
		buffered = malloc(sizeof(struct journal_entry));
		if(!buffered)
		{
			return NULL;
		}
		buffered->rbuffer = calloc(sizeof(fs_word), STD_BUFS);
		if(!buffered->rbuffer)
		{
			return NULL;
		}
		buffered->rbuffer_end = buffered->rbuffer + STD_BUFS;
		buffered->rbuffer_cursor = buffered->rbuffer;


		buffered->dbuffer_start = file->pointer;
		buffered->dbuffer_eof = buffered->dbuffer_start + file->size;
		buffered->dbuffer_eos = buffered->dbuffer_start + file->size;
		buffered->dbuffer_cursor = buffered->dbuffer_start;

	}

	// setup the in-ram buffer
	read_allocated_buffer(buffered->dbuffer_cursor, STD_BUFS, &(buffered->rbuffer));
	return buffered;
}


int __file_putc(struct journal_entry * entry, char __c)
{
	if(!(entry->mode & WEN))
	{
		return -1;
	}
	size_t bufs = entry->rbuffer_end - entry->rbuffer;
	if(entry->rbuffer_cursor == entry->rbuffer_end)
	{
		if(entry->dbuffer_cursor + bufs > entry->dbuffer_eos)
		{
			return 2;
		}

		write_buffer(entry->dbuffer_cursor, bufs, &(entry->rbuffer));
		entry->dbuffer_cursor += bufs;
		entry->rbuffer_cursor = entry->rbuffer;
		read_allocated_buffer(entry->dbuffer_cursor, bufs, &(entry->rbuffer));
	}
	*(entry->rbuffer_cursor++) = __c;
	return 0;

}

char __file_getc(struct journal_entry * entry)
{
	if(!(entry->mode & REN))
	{
		return EOF;
	}

}

