#include<io.h>
#include<string.h>
#include<stdlib.h>
#include<fs/table.h>
#include<fs/inode.h>


struct sjfs_table * load_table(offset_t offset)
{
	struct sjfs_table * table = calloc(sizeof(struct sjfs_table), 1);
	read_allocated_buffer(offset, sizeof(struct sjfs_table), (fs_word **) &table);
	return table;
}


struct inode_descriptor * get_inode_close_to(struct sjfs_table * table, struct inode_descriptor * inode)
{
	// offset_t is unsigned, so -1 is the biggest offset.
	offset_t close_by = (offset_t) -1;
	struct inode_descriptor * current_descriptor, * closest_descriptor = NULL;
	
	offset_t pointer = table->inode_descriptor_table_pointer;
	read_unallocated_buffer(pointer, sizeof(struct inode_descriptor), (fs_word **) &current_descriptor);



	while( pointer <= table->last_inode_descriptor)
	{
		if((inode->pointer < current_descriptor->pointer)
				&& (current_descriptor->pointer < close_by))
		{
			close_by = current_descriptor->pointer;
			if(closest_descriptor != NULL)
			{
				free(closest_descriptor);
			}
			memcpy(closest_descriptor, current_descriptor, sizeof(struct inode_descriptor));
		}
		pointer += sizeof(struct inode_descriptor);
		read_allocated_buffer(pointer, sizeof(struct inode_descriptor), (fs_word **) &current_descriptor); 

	}
	return closest_descriptor;
}

offset_t get_free_inode_table_space(struct sjfs_table * table, size_t requested_size)
{
	struct inode_descriptor * current_descriptor = malloc(sizeof(struct inode_descriptor)), * close_inode = NULL;
	offset_t pointer = table->inode_descriptor_table_pointer;
	offset_t end_of_space = 0;
	do
	{
		read_allocated_buffer(pointer, sizeof(struct inode_descriptor), (fs_word **) &current_descriptor);
		pointer += sizeof(struct inode_descriptor);
		close_inode = get_inode_close_to(table, current_descriptor);
		// we will need this if the next free space is at the end of the inode table
		if(current_descriptor->pointer + current_descriptor->size > end_of_space)
		{
			end_of_space = current_descriptor->pointer + current_descriptor->size;
		}
		// last inode
		if(close_inode == NULL)
		{
			free(current_descriptor);
			return end_of_space;
		}
		if(((current_descriptor->pointer - close_inode->pointer) - current_descriptor->size) >= requested_size)
		{
			offset_t free_space_begin = current_descriptor->pointer + current_descriptor->size;
			free(close_inode);
			free(current_descriptor);
			return free_space_begin;
		}
		free(close_inode);	
	}
	while( pointer <= table->last_inode_descriptor);
	free(current_descriptor);
	return end_of_space;
	
}


// XXX DOES NOT WORK WITH struct link! XXX
struct inode_descriptor * get_file_close_to(struct sjfs_table * table, struct inode_descriptor * inode_descriptor)
{
	// offset_t is unsigned, so -1 is the biggest offset.
	offset_t close_by = (offset_t) -1;
	struct inode_descriptor * current_descriptor, * closest_descriptor = NULL;
	struct dummy_inode * current_inode; 
	
	offset_t pointer = table->inode_descriptor_table_pointer;
	read_unallocated_buffer(pointer, sizeof(struct inode_descriptor), (fs_word **) &current_descriptor);


	// Calculate the end of the current file
	struct dummy_inode * dummy_inode;
	get_dummy_inode(inode_descriptor, &dummy_inode);


	while( pointer <= table->last_inode_descriptor)
	{
		// dereference the inode
		get_dummy_inode(current_descriptor, &current_inode);

		if((dummy_inode->pointer < current_inode->pointer)
				&& (current_inode->pointer < close_by))
		{
			close_by = current_inode->pointer;
			if(closest_descriptor != NULL)
			{
				free(closest_descriptor);
			}
			memcpy(closest_descriptor, current_descriptor, sizeof(struct inode_descriptor));
		}
		pointer += sizeof(struct inode_descriptor);
		read_allocated_buffer(pointer, sizeof(struct inode_descriptor), (fs_word **) &current_descriptor); 
		free(current_inode);

	}
	free(dummy_inode);
	return closest_descriptor;

}


offset_t get_free_file_space(struct sjfs_table * table, size_t requested_size)
{

	struct inode_descriptor * current_descriptor = malloc(sizeof(struct inode_descriptor)), * close_inode_descriptor = NULL;
	struct dummy_inode * current_inode, * close_inode = NULL;
	offset_t pointer = table->inode_descriptor_table_pointer;
	offset_t end_of_space = 0;
	do
	{
		read_allocated_buffer(pointer, sizeof(struct inode_descriptor), (fs_word **) &current_descriptor);
		pointer +=  sizeof(struct inode_descriptor);
		// links do not have any filespace. skip them
		if(current_descriptor->typ == 'L')
		{
			continue;
		}
		get_dummy_inode(current_descriptor, &current_inode);

		close_inode_descriptor = get_file_close_to(table, current_descriptor);

		// we will need this if the next free space is at the end of the inode table
		if(current_inode->pointer + current_inode->size > end_of_space)
		{
			end_of_space = current_inode->pointer + current_inode->size;
		}
		// last inode
		if(close_inode == NULL)
		{
			free(current_descriptor);
			free(current_inode);
			return end_of_space;
		}
		get_dummy_inode(close_inode_descriptor, &close_inode);


		if(((current_inode->pointer - close_inode->pointer) - current_inode->size) >= requested_size)
		{
			offset_t free_space_begin = current_descriptor->pointer + current_descriptor->size;
			free(close_inode);
			free(close_inode_descriptor);
			free(current_inode);
			free(current_descriptor);
			return free_space_begin;
		}
		free(close_inode);	
		free(close_inode_descriptor);
		free(current_inode);
	}
	while( pointer <= table->last_inode_descriptor);
	free(current_descriptor);
	if(end_of_space + requested_size > table->size)
	{
		end_of_space = 0;
	}
	return end_of_space;
	
}
offset_t get_inode_descriptor_offset_by_name(struct sjfs_table * table, char * name)
{
	offset_t pointer = table->inode_descriptor_table_pointer;
	struct inode_descriptor * descriptor = malloc(sizeof(struct inode_descriptor));
	struct directory * dir;
	while(pointer <= table->last_inode_descriptor)
	{
		read_allocated_buffer(pointer, sizeof(struct inode_descriptor), (fs_word **) &descriptor);
		if(descriptor->typ != 'D')
		{
			continue;
		}

		fs_word * buffer = malloc(sizeof(fs_word) * descriptor->size);
		read_allocated_buffer(descriptor->pointer, descriptor->size, &buffer);
		buffer_to_directory(buffer, &dir);
		if(strcmp(name, dir->name) == 0)
		{
			free(dir->name);
			free(dir);
			offset_t res = descriptor->pointer;
			free(descriptor);
			return res;
		}
		free(dir->name);
		free(dir);
		pointer += sizeof(struct inode_descriptor);

	}
	free(descriptor);
	return 0;
}
