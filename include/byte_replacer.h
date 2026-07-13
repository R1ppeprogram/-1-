#ifndef BYTE_REPLACER_H
#define BYTE_REPLACER_H

#include <stdio.h>
#include <stddef.h>


typedef struct {
    unsigned char *data;    // указатель на данные
    size_t size;            // размер в байтах
} ByteSequence;


int replace_bytes_in_file(const char *input_path, 
                         const char *output_path,
                         const ByteSequence *old_seq,
                         const ByteSequence *new_seq,
                         size_t block_size);


int compare_bytes(const unsigned char *buf1, const unsigned char *buf2, size_t len);

#endif // BYTE_REPLACER_H