#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<stdlib.h>
#include<errno.h>

#define FILE_MODE
#ifdef FILE_MODE
char * fname = "fs_test.img";
char * file_buffer = NULL;
#define IMG_SIZE 1024 * 1024 * 1400
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
	if(file_buffer == NULL)
	{
		file_buffer = malloc(sizeof(char) *IMG_SIZE);

		FILE * f = fopen(fname, "r");
		fread(file_buffer, 1,IMG_SIZE, f);
		printf("errno: %d\n", errno);
		fclose(f);
	}
	size_t end = bufs + start;
	if(end >IMG_SIZE)
	{
		end =IMG_SIZE;
	}
	end -= start;
	printf("[__read]{req %zu}{avail %zu}\n", bufs, end);
	memcpy(*buffer, file_buffer + start, end);
	return end;
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
	if(file_buffer == NULL)
	{
		file_buffer = malloc(sizeof(char) *IMG_SIZE);

		FILE * f = fopen(fname, "r");
		fread(file_buffer, 1,IMG_SIZE, f);
		printf("errno: %d\n", errno);
		fclose(f);
	}
	size_t end = bufs + start;
	if(end >IMG_SIZE)
	{
		end =IMG_SIZE;
	}
	end -= start;
	printf("[__write]{req %zu}{avail %zu}\n", bufs, end);
	memcpy(file_buffer + start, *buffer, end);
	// take a look at man fwrite
	FILE * f = fopen(fname, "w");
	size_t written = fwrite(file_buffer, sizeof(fs_word),IMG_SIZE, f) * sizeof(fs_word);
	fclose(f);
	return end;
#endif

}

