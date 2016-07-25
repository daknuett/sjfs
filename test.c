#include<io.h>
#include<stdlib.h>
#include<stdio.h>
#include<unbuffered/fs.h>
#include<unbuffered/directory.h>
#include<unbuffered/file.h>
#include<journal/journal.h>



int main(void)
{
	write_buffer = &__write_buffer;
	read_allocated_buffer = &__read_allocated_buffer;
	read_unallocated_buffer = &__read_unallocated_buffer;
	//fs_word * fs_setup = calloc(sizeof(fs_word) , 512);

	struct sjfs_table * table = load_table(512);
	printf("SJFS TABLE:\n \tUUIDLOW: %lu\n",table->uuid_low);
	printf("\tUUIDHIGH: %lu\n",table->uuid_high);
	printf("\tsize: %lu\n", table->size);
	printf("\tjournal size: %lu\n", table->journal_size);
	printf("\tidt size: %lu\n", table->inode_descriptor_table_size);
	printf("\tit size: %lu\n", table->inode_table_size);

	create_journal_fs(table);
	struct directory * root = get_root(table);
	mkdir(table, root, "test2", 1, 1, 0666);
	mkdir(table, root, "foo", 1, 1, 0777);
	touch_file(table, root, "foo.tx",100, 1, 1, 0755);

	printf("root->size: %zu\n", root->size);
	
	free(root);
	free(table);
	return 0;
}
