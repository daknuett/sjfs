#ifndef __JOURNAL_JOURNAL_H_
#define __JOURNAL_JOURNAL_H_

#include<io.h>
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

void create_journal_fs(struct sjfs_table * table);
struct journal_entry * get_journal_entry_close_to(struct journal_entry_holder * root, struct journal_entry * entry);
char get_journal_buffer(struct sjfs_table * table, size_t requested_size, struct buffer_info ** buffer);
struct journal_entry * buffer_directory(struct sjfs_table * table, struct directory * dir, size_t additional_buffer, char mode);
struct journal_entry * buffer_file(struct sjfs_table * table, struct file * fil, size_t additional_buffer, char mode);
void register_entry(struct journal_entry_holder * root, struct journal_entry * entry);
void unregister_entry(struct journal_entry_holder * root, struct journal_entry * entry);
char unbuffer_entry(struct sjfs_table * table, struct journal_entry * entry);
#endif
