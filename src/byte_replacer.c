#define _CRT_SECURE_NO_WARNINGS
#include "byte_replacer.h"
#include <stdlib.h>
#include <string.h>


static size_t find_match(const unsigned char *buffer, size_t buf_size,
                         const ByteSequence *pattern, size_t start_pos,
                         unsigned char *partial_match, size_t *partial_size) {
    // Поиск с учётом частичных совпадений на границе блоков
    size_t i;
    
    if (*partial_size > 0) {
        // Проверяем, не является ли начало буфера продолжением частичного совпадения
        size_t check_len = pattern->size - *partial_size;
        if (check_len <= buf_size && 
            memcmp(partial_match, pattern->data, *partial_size) == 0 &&
            memcmp(buffer, pattern->data + *partial_size, check_len) == 0) {
            return 0; // Найдено совпадение на границе
        }
        *partial_size = 0;
    }
    
    for (i = start_pos; i <= buf_size - pattern->size; i++) {
        if (memcmp(buffer + i, pattern->data, pattern->size) == 0) {
            return i;
        }
    }
    
    // Сохраняем частичное совпадение на конце буфера
    if (pattern->size > 1) {
        for (i = buf_size - 1; i > 0 && i >= buf_size - pattern->size + 1; i--) {
            size_t suffix_len = buf_size - i;
            if (suffix_len < pattern->size && 
                memcmp(buffer + i, pattern->data, suffix_len) == 0) {
                // Проверяем, является ли это началом совпадения
                if (*partial_size == 0) {
                    memcpy(partial_match, buffer + i, suffix_len);
                    *partial_size = suffix_len;
                }
            }
        }
    }
    
    return (size_t)-1; // Не найдено
}

int replace_bytes_in_file(const char *input_path, 
                         const char *output_path,
                         const ByteSequence *old_seq,
                         const ByteSequence *new_seq,
                         size_t block_size) {
    FILE *in = NULL, *out = NULL;
    unsigned char *buffer = NULL;
    unsigned char *output_buffer = NULL;
    size_t bytes_read;
    size_t output_pos = 0;
    unsigned char partial_match[256]; // Буфер для частичных совпадений
    size_t partial_size = 0;
    size_t processed = 0;
    int result = -1;
    
    // Проверка валидности параметров
    if (!input_path || !output_path || !old_seq || !new_seq || 
        old_seq->size == 0 || old_seq->size > 2 * block_size) {
        return -1;
    }
    
    // Открываем файлы
    in = fopen(input_path, "rb");
    if (!in) return -1;
    
    out = fopen(output_path, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }
    
    // Выделяем буферы (не более 4*N)
    buffer = (unsigned char*)malloc(block_size);
    output_buffer = (unsigned char*)malloc(block_size * 2);
    if (!buffer || !output_buffer) {
        goto cleanup;
    }
    
    while ((bytes_read = fread(buffer, 1, block_size, in)) > 0) {
        size_t pos = 0;
        
        while (pos < bytes_read) {
            // Ищем совпадение
            size_t match_pos = find_match(buffer, bytes_read, old_seq, pos,
                                         partial_match, &partial_size);
            
            if (match_pos == (size_t)-1) {
                // Нет совпадений, копируем оставшиеся байты
                size_t remaining = bytes_read - pos;
                // Учитываем, что некоторые байты могут быть частью частичного совпадения
                if (remaining > old_seq->size - 1 && partial_size == 0) {
                    // Копируем все оставшиеся байты
                    memcpy(output_buffer + output_pos, buffer + pos, remaining);
                    output_pos += remaining;
                    pos = bytes_read;
                } else {
                    // Не копируем байты, которые могут быть частью совпадения
                    break;
                }
            } else {
                // Нашли совпадение
                // Сначала копируем байты до совпадения
                if (match_pos > pos) {
                    memcpy(output_buffer + output_pos, buffer + pos, match_pos - pos);
                    output_pos += match_pos - pos;
                }
                
                // Записываем замену
                if (output_pos + new_seq->size > block_size * 2) {
                    // Буфер заполнен, сбрасываем
                    if (fwrite(output_buffer, 1, output_pos, out) != output_pos) {
                        goto cleanup;
                    }
                    output_pos = 0;
                }
                memcpy(output_buffer + output_pos, new_seq->data, new_seq->size);
                output_pos += new_seq->size;
                
                pos = match_pos + old_seq->size;
                partial_size = 0; // Сброс частичного совпадения
            }
        }
        
        // Сохраняем необработанные байты для следующего блока
        processed = bytes_read;
    }
    
    // Записываем оставшиеся данные
    if (output_pos > 0) {
        if (fwrite(output_buffer, 1, output_pos, out) != output_pos) {
            goto cleanup;
        }
    }
    
    result = 0;
    
cleanup:
    free(buffer);
    free(output_buffer);
    if (in) fclose(in);
    if (out) fclose(out);
    return result;
}

int compare_bytes(const unsigned char *buf1, const unsigned char *buf2, size_t len) {
    return memcmp(buf1, buf2, len);
}