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

	sjfs_build_table(512,
			0x2323, 0x2323, 
			400 * 1024* 1024,
			2048, 65536,
			1468005888);

	struct sjfs_table * table = load_table(512);
	printf("SJFS TABLE:\n \tUUIDLOW: %lu\n",table->uuid_low);
	printf("\tUUIDHIGH: %lu\n",table->uuid_high);
	printf("\tsize: %lu\n", table->size);
	printf("\tjournal size: %lu\n", table->journal_size);
	printf("\tidt size: %lu\n", table->inode_descriptor_table_size);
	printf("\tit size: %lu\n", table->inode_table_size);
	mkroot(table);

	create_journal_fs(table);
	struct directory * root = get_root(table);
	char success = touch_file(table, root, "foo.tx", 30, 1000, 1000, 0777);	
	printf("created file (ok = %d)\n", success);

	offset_t id_offset= get_inode_descriptor_offset_by_name(table, "foo.tx");
	struct inode_descriptor * file_desc;
	read_unallocated_buffer(id_offset, sizeof(struct inode_descriptor), (fs_word **) &file_desc);
	printf("%x\n", file_desc);
	fs_word * _file_buffer;
	read_unallocated_buffer(file_desc->pointer, file_desc->size, &_file_buffer);
	printf("%s: %d\n", __FILE__, __LINE__);
//	free(file_desc);
	struct file * file;
	printf("%s: %d\n", __FILE__, __LINE__);
	buffer_to_file(_file_buffer, &file);
	printf("%s: %d\n", __FILE__, __LINE__);
	free(_file_buffer);

	fs_word * file_mem = calloc(sizeof(fs_word), file->size);
	read_allocated_buffer(file->pointer, file->size, &file_mem);
	printf("%s: %d\n", __FILE__, __LINE__);

	size_t i;
	printf("file->name: %s\n", file->name);
	printf("%zu\n", file->size);
	for( i = 0; i < file->size; i++)
	{
		printf("%c", file_mem[i]);
	}
	printf("\n");

	/*
	struct journal_entry * my_file = open_file(table, file, MODE_W);
	char str[6] = "hello";
	int i = 0;
	while(str[i])
	{
		printf("putting %c [res: %d]\n", str[i], __file_putc(my_file, str[i]));
		i++;
	}
	*/
	
	free(root);
	free(table);
	return 0;
}
