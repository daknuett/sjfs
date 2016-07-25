#include<fs/table.h>
#include<fs/inode.h>
#include<fs_types.h>
#include<io.h>


void sjfs_build_table(offset_t offset,
		uint64_t uuid_low, uint64_t uuid_high,
		offset_t journal_size,
		offset_t inode_descriptor_table_size,
		offset_t inode_table_size,
		offset_t size)
{
	struct sjfs_table * table = calloc(sizeof(struct sjfs_table), 1);
	table->uuid_low = uuid_low;
	table->uuid_high = uuid_high;
	table->typ = 8;
	table->journal_size = journal_size;
	table->inode_descriptor_table_size = inode_descriptor_table_size;
	table->inode_table_size = inode_table_size;
	
	// calculate pointers
	// the sjfs_table
	size_t base_pointer = 89;
	// journal_pointer
	size_t pointer = offset + base_pointer;
	table->journal_pointer = pointer;
	pointer += journal_size;
	table->inode_descriptor_table_pointer = pointer;
	pointer += inode_descriptor_table_size;
	table->inode_table_pointer = pointer;
	pointer += inode_table_size;
	table->file_space_pointer = pointer;
	table->size = size - (pointer - offset);
	
	(*write_buffer)(offset, sizeof(struct sjfs_table), (fs_word **) &table);
	free(table);

}



