#include<io.h>
#include<fs/table.h>
#include<fs/inode.h>
#include<unbuffered/fs.h>

#define WEN		0b0010
#define	REN		0b0001
#define TRUNCATE	0b0100
#define	CURSOR_END	0b1000
#define MODE_R 		0b0001
#define MODE_W 		0b0110
#define MODE_RPLUS	0b0011
#define MODE_WPLUS	0b0111
#define MODE_A 		0b1110
/*			  ^^^^
      cursor_set to end --||||
      trunctate file    ---|||
      write enable      ----||
      read enable       -----|
	*/

struct journal_entry
{
	offset_t dbuffer_start,
		 dbuffer_cursor,
		 dbuffer_eof,
		 dbuffer_eos;
	offset_t inode_descriptor;
	fs_word * rbuffer;
	fs_word * rbuffer_end;
	fs_word * rbuffer_cursor;
	char mode; // open for r/w
	char in_journal:1; // bool
};

struct buffer_info
{
	offset_t buffer_start,
		 buffer_end;
	size_t max_buffer_size,
	       requested_buffer_size;
	char in_journal:1;
};

struct journal_entry_holder
{
	struct journal_entry * entry;
	struct journal_entry_holder * next;
};
struct journal_entry_holder * get_journal_root(struct sjfs_table * table)
{
	size_t pointer = 0, *ptr = &pointer;
	struct journal_entry_holder * root;
	read_allocated_buffer(table->journal_pointer, sizeof(size_t), (fs_word **) &ptr);
	root = (struct journal_entry_holder *) pointer;
	return root;
}

void create_journal_fs(struct sjfs_table * table)
{
	// we will save the address of the first journal_entry_holder
	struct journal_entry_holder * root = malloc(sizeof(struct journal_entry_holder));
	root->next = NULL;
	root->entry = NULL;
	size_t pointer = (size_t ) root;
	fs_word * ptr =(fs_word *) &pointer;
	write_buffer(table->journal_pointer, sizeof(size_t), &ptr);
}
struct journal_entry * get_journal_entry_close_to(struct journal_entry_holder * root, struct journal_entry * entry)
{
	struct journal_entry * closest = NULL;
	struct journal_entry_holder * current = root;
	while(current->next != NULL)
	{
		printf("current: %lu\n", (long unsigned int) current);
		if(current->entry == NULL)
		{
			current = current->next;
			continue;
		}
		if((current->entry->dbuffer_eos > entry->dbuffer_eos))
		{
			if(closest == NULL)
			{
				closest = current->entry;
			}
			else
			{
				if(closest->dbuffer_eos > current->entry->dbuffer_eos)
				{
					closest = current->entry;
				}
			}
		}
		current = current->next;
	}
	return closest;
}
char get_journal_buffer(struct sjfs_table * table, size_t requested_size, struct buffer_info ** buffer)
{
	// FIXME might not work correctly!
	// get the first journal_entry_holder
	struct journal_entry_holder * root;
	struct buffer_info * buf = *buffer;
	root = get_journal_root(table);
	// now root is the journal_entry_holder we allocated in create_journal_fs

	// empty journal
	if(root->next == NULL)
	{
		if(requested_size <= table->journal_size)
		{
			buf->buffer_start = table->journal_pointer + sizeof(size_t);
			buf->buffer_end = table->journal_pointer + sizeof(size_t) + requested_size;
			buf->max_buffer_size = table->journal_size;
			buf->requested_buffer_size = requested_size;
			buf->in_journal = 1;
			return 0;

		}
		else
		{
			// file too big for journal
			offset_t file_space = get_free_file_space(table, requested_size);
			// check if the filr fits into filespace
			if(file_space + requested_size <= table->file_space_pointer + table->size)
			{
				buf->buffer_start = file_space;
				buf->buffer_end = file_space + requested_size;
				// FIXME: calculate the actual max_buffer_size
				buf->max_buffer_size = requested_size;
				buf->requested_buffer_size = requested_size;
				buf->in_journal = 0;
				return 0;

			}
			else
			{
				// cannot buffer the file!
				return 1;
			}
		}
	}
	else
	{
		// we will need to search free space
		// this will be the offset of the last element + its size
		offset_t end_of_files = 0;
		struct journal_entry_holder * current_entry_holder = root;
		struct journal_entry * next_entry;
		while(current_entry_holder->next != NULL)
		{
			if(current_entry_holder->entry == NULL)
			{
				current_entry_holder = current_entry_holder->next;
				continue;
			}
			if(current_entry_holder->entry->dbuffer_eos > end_of_files)
			{
				end_of_files = current_entry_holder->entry->dbuffer_eos;
			}
			next_entry = get_journal_entry_close_to(root, current_entry_holder->entry);
			if(next_entry == NULL)
			{
				continue;
			}
			// we found the space
			if(current_entry_holder->entry->dbuffer_eos + requested_size < next_entry->dbuffer_start)
			{
				buf->buffer_start = current_entry_holder->entry->dbuffer_eos;
				buf->max_buffer_size = next_entry->dbuffer_start - 1;
				buf->buffer_end = current_entry_holder->entry->dbuffer_eos + requested_size;
				buf->requested_buffer_size = requested_size;
				buf->in_journal = 1;
				return 0;
			}
			current_entry_holder = current_entry_holder->next;

		}
		// our buffer might be at the end of the journal, but we will have to 
		// check for the size of the journal
		if(end_of_files + requested_size <= table->journal_pointer + table->journal_size)
		{

			buf->buffer_start = end_of_files;
			buf->max_buffer_size = table->journal_size - end_of_files;
			buf->buffer_end = end_of_files + requested_size;
			buf->requested_buffer_size = requested_size;
			buf->in_journal = 1;
			return 0;
		}
		// hmm we need space from the filespace.
		offset_t file_space = get_free_file_space(table, requested_size);
		// check if the filr fits into filespace
		if(file_space + requested_size <= table->file_space_pointer + table->size)
		{
			buf->buffer_start = file_space;
			buf->buffer_end = file_space + requested_size;
			// FIXME: calculate the actual max_buffer_size
			buf->max_buffer_size = requested_size;
			buf->requested_buffer_size = requested_size;
			buf->in_journal = 0;
			return 0;

		}
		else
		{
			// cannot buffer the file!
			return 1;
		}
	}

}
void register_entry(struct journal_entry_holder * root, struct journal_entry * entry)
{
	if(!entry->in_journal)
	{
		return;
	}
	struct journal_entry_holder * current = root;
	while(current->next != NULL)
	{
		current = current->next;
	}
	struct journal_entry_holder * holder = calloc(sizeof(struct journal_entry_holder), 1);
	holder->entry = entry;
	current->next = holder;

}

struct journal_entry * buffer_directory(struct sjfs_table * table, struct directory * dir, size_t additional_buffer, char mode)
{
	struct buffer_info * buff = malloc(sizeof(struct buffer_info));
	char buffer_not_ok = get_journal_buffer(table, dir->size + additional_buffer, &buff);
	if(buffer_not_ok)
	{
		goto exit_nospace;
	}
	struct journal_entry * entry = calloc(sizeof(struct journal_entry), 1);
	if(!entry)
	{
		goto exit_nospace;
	}
	entry->rbuffer = calloc(sizeof(fs_word), additional_buffer);
	if(!entry->rbuffer)
	{
exit_nospace:
		free(buff);
		return NULL;
	}

	entry->rbuffer_end = entry->rbuffer + additional_buffer;
	entry->rbuffer_cursor = entry->rbuffer;
	entry->dbuffer_start = buff->buffer_start;
	entry->dbuffer_eof = buff->buffer_start + dir->size;
	entry->dbuffer_eos = buff->buffer_end;
	entry->dbuffer_cursor = buff->buffer_start;
	entry->mode = mode;
	entry->in_journal = buff->in_journal;
	entry->inode_descriptor = get_inode_descriptor_offset_by_name(table, dir->name);

	// copy content
	size_t left = dir->size,
	       chunk_size;
	offset_t ptr = dir->pointer;
	while(left)
	{
		if(left < additional_buffer)
		{
			chunk_size = left;
		}
		else
		{
			chunk_size = additional_buffer;
		}
		read_allocated_buffer(ptr, chunk_size, &entry->rbuffer);
		write_buffer(entry->dbuffer_cursor, chunk_size, &entry->rbuffer);
		ptr += chunk_size;
		left -=	chunk_size;
		entry->dbuffer_cursor += chunk_size;

	}
	free(buff);
	register_entry(get_journal_root(table), entry);
	return entry;
}



struct journal_entry * buffer_file(struct sjfs_table * table, struct file * fil, size_t additional_buffer, char mode)
{
	struct buffer_info * buff = malloc(sizeof(struct buffer_info));
	char buffer_not_ok;
	if(mode & TRUNCATE)
	{
		buffer_not_ok = get_journal_buffer(table, additional_buffer, &buff);
	}
	else
	{
		buffer_not_ok = get_journal_buffer(table, fil->size + additional_buffer, &buff);
	}
	if(buffer_not_ok)
	{
		goto exit_nospace;
	}
	struct journal_entry * entry = calloc(sizeof(struct journal_entry), 1);
	if(!entry)
	{
		goto exit_nospace;
	}
	entry->rbuffer = calloc(sizeof(fs_word), additional_buffer);
	if(!entry->rbuffer)
	{
exit_nospace:
		free(buff);
		return NULL;
	}

	entry->rbuffer_end = entry->rbuffer + additional_buffer;
	entry->rbuffer_cursor = entry->rbuffer;
	entry->dbuffer_start = buff->buffer_start;
	entry->dbuffer_eof = buff->buffer_start + fil->size;
	entry->dbuffer_eos = buff->buffer_end;
	entry->dbuffer_cursor = buff->buffer_start;
	entry->mode = mode;
	entry->in_journal = buff->in_journal;
	entry->inode_descriptor = get_inode_descriptor_offset_by_name(table, fil->name);

	if(!(mode & TRUNCATE))
	{
		// copy content
		size_t left = fil->size,
		       chunk_size;
		offset_t ptr = fil->pointer;
		while(left)
		{
			if(left < additional_buffer)
			{
				chunk_size = left;
			}
			else
			{
				chunk_size = additional_buffer;
			}
			read_allocated_buffer(ptr, chunk_size, &entry->rbuffer);
			write_buffer(entry->dbuffer_cursor, chunk_size, &entry->rbuffer);
			ptr += chunk_size;
			left -=	chunk_size;
			entry->dbuffer_cursor += chunk_size;

		}
		if(!( mode & CURSOR_END))
		{
			entry->dbuffer_cursor =  entry->dbuffer_start;
		}
	}
	free(buff);
	register_entry(get_journal_root(table), entry);
	return entry;
}

void unregister_entry(struct journal_entry_holder * root, struct journal_entry * entry)
{
	free(entry->rbuffer);
	if(!entry->in_journal)
	{
		free(entry);
		return;
	}
	struct journal_entry_holder * last = root, * current = root->next;
	
	do
	{
		if(current->entry == entry)
		{
			last->next = current->next;
			free(current);
			free(entry->rbuffer);
			free(entry);
			break;
		}
		last = current;
		current = current->next;
	}while(current->next != NULL);

}

char unbuffer_entry(struct sjfs_table * table, struct journal_entry * entry)
{
	// actually a pseudo entry.
	// no action will be performed
	if(!entry->mode & WEN)
	{
		free(entry->rbuffer);
		free(entry);
		return 0;
	}
	// we are in filespace. no need to copy.
	// just forget the old allocated space
	if(!entry->in_journal)
	{
		struct inode_descriptor * descriptor;
		read_unallocated_buffer(entry->inode_descriptor, sizeof(struct inode_descriptor), (fs_word **) &descriptor);
		struct dummy_inode * dir;
		get_dummy_inode(descriptor, &dir);
		dir->pointer = entry->dbuffer_start;
		dir->size = entry->dbuffer_eof - entry->dbuffer_start;
		write_buffer(descriptor->pointer, sizeof(struct dummy_inode),(fs_word **) &dir);
		free(descriptor);
		free(dir);
		unregister_entry(get_journal_root(table), entry);
		
	}
	else
	{
		size_t size = entry->dbuffer_eof - entry->dbuffer_start;
		// 
		struct inode_descriptor * descriptor;
		read_unallocated_buffer(entry->inode_descriptor, sizeof(struct inode_descriptor), (fs_word **) &descriptor);
		struct dummy_inode * dir;
		get_dummy_inode(descriptor, &dir);
		dir->size = 0; 
		// if there is enough space after our current file this
		// will allow us to use that
		write_buffer(descriptor->pointer, sizeof(struct dummy_inode),(fs_word **) &dir);

		offset_t free_space = get_free_file_space(table, size);
		dir->size = size;
		if(free_space +  size > table->file_space_pointer + table->size)
		{
			return -1;
		}
		dir->pointer = free_space;
		write_buffer(descriptor->pointer, sizeof(struct dummy_inode),(fs_word **) &dir);

		// copy the content

		size_t left = size,
		       chunk_size,
		       bufs = entry->rbuffer_end - entry->rbuffer;
		offset_t ptr = free_space;
		entry->dbuffer_cursor = entry->dbuffer_start;
		while(left)
		{
			if(left < bufs)
			{
				chunk_size = left;
			}
			else
			{
				chunk_size = bufs;
			}
		//	printf("left:%zu copy chunk of size %zu\n", left, chunk_size);
			read_allocated_buffer(entry->dbuffer_cursor, chunk_size, &entry->rbuffer);
			write_buffer(ptr, chunk_size, &entry->rbuffer);
			ptr += chunk_size;
			left -=	chunk_size;
			entry->dbuffer_cursor += chunk_size;

		}
		unregister_entry(get_journal_root(table), entry);
	}
}
