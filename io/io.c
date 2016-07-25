#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>

#define FILE_MODE
#ifdef FILE_MODE
char * fname = "fs_test.img";
#endif




typedef uint64_t offset_t;
typedef unsigned char fs_word;

size_t (* read_allocated_buffer)(offset_t, size_t, fs_word **);
size_t (* read_unallocated_buffer)(offset_t, size_t, fs_word **);
size_t (* write_buffer)(offset_t, size_t, fs_word **);
size_t __read_allocated_buffer(offset_t start, size_t bufs, fs_word ** buffer);
size_t __read_unallocated_buffer(offset_t start, size_t bufs, fs_word ** unallocated_buffer);
size_t __write_buffer(offset_t start, size_t bufs, fs_word ** buffer);


size_t __read_allocated_buffer(offset_t start, size_t bufs, fs_word ** buffer)
{
#ifdef FILE_MODE
	FILE * f = fopen(fname, "r");
	fseek(f, (long int) start, SEEK_SET);
	size_t read = 0;
	fs_word _c = fgetc(f);
	fs_word * b = * buffer;
	while(_c != EOF && read < bufs)
	{
		b[read++] = _c;
		_c = fgetc(f);
	}
	fclose(f);
	return read;
#endif
	// other modes maybe later...
}



size_t __read_unallocated_buffer(offset_t start, size_t bufs, fs_word ** unallocated_buffer)
{
	fs_word * buffer = calloc(sizeof(fs_word), bufs);
	*unallocated_buffer = buffer;
	return __read_allocated_buffer(start, bufs, &buffer);
}


size_t __write_buffer(offset_t start, size_t bufs, fs_word ** buffer)
{
#ifdef FILE_MODE
	FILE * f = fopen(fname, "r+");
	fseek(f, (long int) start, SEEK_SET);
	// take a look at man fwrite
	size_t written = fwrite(*buffer, sizeof(fs_word), bufs, f) * sizeof(fs_word);
	fclose(f);
	return written;
#endif

}

