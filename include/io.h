#ifndef __IO_H_
#define __IO_H_
#include<stdlib.h>
#include<stdint.h>
#include<fs_types.h>

extern size_t (* read_allocated_buffer)(offset_t, size_t, fs_word **);
extern size_t (* read_unallocated_buffer)(offset_t, size_t, fs_word **);
extern size_t (* write_buffer)(offset_t, size_t, fs_word **);

size_t __read_allocated_buffer(offset_t start, size_t bufs, fs_word ** buffer);
size_t __read_unallocated_buffer(offset_t start, size_t bufs, fs_word ** unallocated_buffer);
size_t __write_buffer(offset_t start, size_t bufs, fs_word ** buffer);
#endif
