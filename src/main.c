#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "byte_replacer.h"

// Функция для преобразования строки в байтовую последовательность
// Поддерживает: обычный текст, hex-формат, экранирование
static int parse_byte_sequence(const char *arg, ByteSequence *seq) {
    // Если строка начинается с "hex:", то это hex-кодированные данные
    if (strncmp(arg, "hex:", 4) == 0) {
        const char *hex = arg + 4;
        size_t len = strlen(hex);
        if (len % 2 != 0) return -1;
        
        seq->size = len / 2;
        seq->data = (unsigned char*)malloc(seq->size);
        if (!seq->data) return -1;
        
        for (size_t i = 0; i < seq->size; i++) {
            char byte_str[3] = {hex[i*2], hex[i*2+1], '\0'};
            char *endptr;
            unsigned long val = strtoul(byte_str, &endptr, 16);
            if (*endptr != '\0') {
                free(seq->data);
                return -1;
            }
            seq->data[i] = (unsigned char)val;
        }
        return 0;
    }
    
    // Если строка в кавычках, удаляем кавычки
    size_t len = strlen(arg);
    if (len >= 2 && arg[0] == '"' && arg[len-1] == '"') {
        seq->data = (unsigned char*)malloc(len - 1);
        if (!seq->data) return -1;
        memcpy(seq->data, arg + 1, len - 1);
        seq->size = len - 1;
        return 0;
    }
    
    // Обычная строка
    seq->data = (unsigned char*)malloc(len);
    if (!seq->data) return -1;
    memcpy(seq->data, arg, len);
    seq->size = len;
    return 0;
}

static void free_sequence(ByteSequence *seq) {
    if (seq->data) {
        free(seq->data);
        seq->data = NULL;
        seq->size = 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <input_file> <output_file> <old_sequence> <new_sequence>\n", argv[0]);
        fprintf(stderr, "Sequences can be:\n");
        fprintf(stderr, "  - plain text: \"text\"\n");
        fprintf(stderr, "  - hex: hex:414243 (for ABC)\n");
        fprintf(stderr, "  - plain without quotes: text\n");
        return 1;
    }
    
    const char *input_path = argv[1];
    const char *output_path = argv[2];
    
    ByteSequence old_seq = {NULL, 0};
    ByteSequence new_seq = {NULL, 0};
    
    // Парсим последовательности
    if (parse_byte_sequence(argv[3], &old_seq) != 0) {
        fprintf(stderr, "Error: invalid old sequence format\n");
        return 1;
    }
    
    if (parse_byte_sequence(argv[4], &new_seq) != 0) {
        fprintf(stderr, "Error: invalid new sequence format\n");
        free_sequence(&old_seq);
        return 1;
    }
    
    // Проверяем размер старой последовательности
    // N задаётся при компиляции через -DBLOCK_SIZE=N
    #ifndef BLOCK_SIZE
    #define BLOCK_SIZE 4096
    #endif
    
    if (old_seq.size > 2 * BLOCK_SIZE) {
        fprintf(stderr, "Error: old sequence size (%zu) exceeds limit (2*%d = %d)\n",
                old_seq.size, BLOCK_SIZE, 2 * BLOCK_SIZE);
        free_sequence(&old_seq);
        free_sequence(&new_seq);
        return 1;
    }
    
    printf("Processing: %s -> %s\n", input_path, output_path);
    printf("Replacing sequence of size %zu with size %zu\n", 
           old_seq.size, new_seq.size);
    printf("Block size: %d bytes\n", BLOCK_SIZE);
    
    int result = replace_bytes_in_file(input_path, output_path, 
                                       &old_seq, &new_seq, BLOCK_SIZE);
    
    free_sequence(&old_seq);
    free_sequence(&new_seq);
    
    if (result == 0) {
        printf("Success!\n");
        return 0;
    } else {
        fprintf(stderr, "Error during processing\n");
        return 1;
    }
}