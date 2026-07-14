#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "byte_replacer.h"

// Вспомогательная функция для создания тестового файла
static void create_test_file(const char *path, const unsigned char *data, size_t size) {
    FILE *f = fopen(path, "wb");
    assert(f);
    size_t written = fwrite(data, 1, size, f);
    assert(written == size);
    fclose(f);
}

// Вспомогательная функция для чтения файла
static unsigned char* read_test_file(const char *path, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *data = (unsigned char*)malloc(*size);
    if (!data) {
        fclose(f);
        return NULL;
    }
    size_t read = fread(data, 1, *size, f);
    if (read != *size) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    return data;
}

// Тест 1: базовая замена в середине
void test_basic_replacement() {
    printf("Test 1: Basic replacement... ");
    const unsigned char input[] = "abcdefghij";
    const unsigned char old_seq[] = "def";
    const unsigned char new_seq[] = "XYZ";
    const unsigned char expected[] = "abcXYZghij";
    
    create_test_file("test_input1.bin", input, sizeof(input)-1);
    
    ByteSequence old_s = {(unsigned char*)old_seq, sizeof(old_seq)-1};
    ByteSequence new_s = {(unsigned char*)new_seq, sizeof(new_seq)-1};
    
    int result = replace_bytes_in_file("test_input1.bin", "test_output1.bin", 
                                       &old_s, &new_s, 4);
    assert(result == 0);
    
    size_t out_size;
    unsigned char *out_data = read_test_file("test_output1.bin", &out_size);
    assert(out_data);
    assert(out_size == sizeof(expected)-1);
    assert(memcmp(out_data, expected, out_size) == 0);
    free(out_data);
    
    printf("PASSED\n");
}

// Тест 2: замена в начале
void test_replacement_at_start() {
    printf("Test 2: Replacement at start... ");
    const unsigned char input[] = "abcdefghij";
    const unsigned char old_seq[] = "abc";
    const unsigned char new_seq[] = "12345";
    const unsigned char expected[] = "12345defghij";
    
    create_test_file("test_input2.bin", input, sizeof(input)-1);
    
    ByteSequence old_s = {(unsigned char*)old_seq, sizeof(old_seq)-1};
    ByteSequence new_s = {(unsigned char*)new_seq, sizeof(new_seq)-1};
    
    int result = replace_bytes_in_file("test_input2.bin", "test_output2.bin", 
                                       &old_s, &new_s, 4);
    assert(result == 0);
    
    size_t out_size;
    unsigned char *out_data = read_test_file("test_output2.bin", &out_size);
    assert(out_data);
    assert(out_size == sizeof(expected)-1);
    assert(memcmp(out_data, expected, out_size) == 0);
    free(out_data);
    
    printf("PASSED\n");
}

// Тест 3: замена в конце
void test_replacement_at_end() {
    printf("Test 3: Replacement at end... ");
    const unsigned char input[] = "abcdefghij";
    const unsigned char old_seq[] = "hij";
    const unsigned char new_seq[] = "789";
    const unsigned char expected[] = "abcdefg789";
    
    create_test_file("test_input3.bin", input, sizeof(input)-1);
    
    ByteSequence old_s = {(unsigned char*)old_seq, sizeof(old_seq)-1};
    ByteSequence new_s = {(unsigned char*)new_seq, sizeof(new_seq)-1};
    
    int result = replace_bytes_in_file("test_input3.bin", "test_output3.bin", 
                                       &old_s, &new_s, 4);
    assert(result == 0);
    
    size_t out_size;
    unsigned char *out_data = read_test_file("test_output3.bin", &out_size);
    assert(out_data);
    assert(out_size == sizeof(expected)-1);
    assert(memcmp(out_data, expected, out_size) == 0);
    free(out_data);
    
    printf("PASSED\n");
}

// Тест 4: множественные замены
void test_multiple_replacements() {
    printf("Test 4: Multiple replacements... ");
    const unsigned char input[] = "abcabcabc";
    const unsigned char old_seq[] = "abc";
    const unsigned char new_seq[] = "12";
    const unsigned char expected[] = "121212";
    
    create_test_file("test_input4.bin", input, sizeof(input)-1);
    
    ByteSequence old_s = {(unsigned char*)old_seq, sizeof(old_seq)-1};
    ByteSequence new_s = {(unsigned char*)new_seq, sizeof(new_seq)-1};
    
    int result = replace_bytes_in_file("test_input4.bin", "test_output4.bin", 
                                       &old_s, &new_s, 4);
    assert(result == 0);
    
    size_t out_size;
    unsigned char *out_data = read_test_file("test_output4.bin", &out_size);
    assert(out_data);
    assert(out_size == sizeof(expected)-1);
    assert(memcmp(out_data, expected, out_size) == 0);
    free(out_data);
    
    printf("PASSED\n");
}

// Тест 5: частичное совпадение на границе блока
void test_partial_match_boundary() {
    printf("Test 5: Partial match on block boundary... ");
    const unsigned char input[] = "abcdefghijklmnop";
    const unsigned char old_seq[] = "fghijk";
    const unsigned char new_seq[] = "!!!";
    const unsigned char expected[] = "abcde!!!lmnop";
    
    create_test_file("test_input5.bin", input, sizeof(input)-1);
    
    ByteSequence old_s = {(unsigned char*)old_seq, sizeof(old_seq)-1};
    ByteSequence new_s = {(unsigned char*)new_seq, sizeof(new_seq)-1};
    
    // Используем блок размером 6, чтобы "fgh" оказалось в конце блока
    int result = replace_bytes_in_file("test_input5.bin", "test_output5.bin", 
                                       &old_s, &new_s, 6);
    assert(result == 0);
    
    size_t out_size;
    unsigned char *out_data = read_test_file("test_output5.bin", &out_size);
    assert(out_data);
    assert(out_size == sizeof(expected)-1);
    assert(memcmp(out_data, expected, out_size) == 0);
    free(out_data);
    
    printf("PASSED\n");
}

// Тест 6: вхождение содержит нулевые байты
void test_null_bytes() {
    printf("Test 6: Null bytes in sequence... ");
    unsigned char input[] = {0x41, 0x00, 0x42, 0x00, 0x43, 0x44, 0x45};
    unsigned char old_seq[] = {0x42, 0x00, 0x43};
    unsigned char new_seq[] = {0x58, 0x59, 0x5A};
    unsigned char expected[] = {0x41, 0x00, 0x58, 0x59, 0x5A, 0x44, 0x45};
    
    create_test_file("test_input6.bin", input, sizeof(input));
    
    ByteSequence old_s = {old_seq, sizeof(old_seq)};
    ByteSequence new_s = {new_seq, sizeof(new_seq)};
    
    int result = replace_bytes_in_file("test_input6.bin", "test_output6.bin", 
                                       &old_s, &new_s, 4);
    assert(result == 0);
    
    size_t out_size;
    unsigned char *out_data = read_test_file("test_output6.bin", &out_size);
    assert(out_data);
    assert(out_size == sizeof(expected));
    assert(memcmp(out_data, expected, out_size) == 0);
    free(out_data);
    
    printf("PASSED\n");
}

// Тест 7: последовательность больше блока
void test_sequence_larger_than_block() {
    printf("Test 7: Sequence larger than block... ");
    unsigned char input[] = "abcdefghijklmnopqrstuvwxyz";
    unsigned char old_seq[] = "fghijklmnopqrstu";
    unsigned char new_seq[] = "___";
    unsigned char expected[] = "abcde___vwxyz";
    
    create_test_file("test_input7.bin", input, sizeof(input)-1);
    
    ByteSequence old_s = {old_seq, sizeof(old_seq)-1};
    ByteSequence new_s = {new_seq, sizeof(new_seq)-1};
    
    // Блок меньше старой последовательности (но ≤ 2*N разрешено)
    int result = replace_bytes_in_file("test_input7.bin", "test_output7.bin", 
                                       &old_s, &new_s, 8);
    assert(result == 0);
    
    size_t out_size;
    unsigned char *out_data = read_test_file("test_output7.bin", &out_size);
    assert(out_data);
    assert(out_size == sizeof(expected)-1);
    assert(memcmp(out_data, expected, out_size) == 0);
    free(out_data);
    
    printf("PASSED\n");
}

// Тест 8: замена на пустую последовательность (удаление)
void test_empty_replacement() {
    printf("Test 8: Empty replacement (deletion)... ");
    unsigned char input[] = "abcdefghijklmnop";
    unsigned char old_seq[] = "fghijk";
    unsigned char new_seq[] = "";
    unsigned char expected[] = "abcdelmnop";
    
    create_test_file("test_input8.bin", input, sizeof(input)-1);
    
    ByteSequence old_s = {old_seq, sizeof(old_seq)-1};
    ByteSequence new_s = {new_seq, 0};
    
    int result = replace_bytes_in_file("test_input8.bin", "test_output8.bin", 
                                       &old_s, &new_s, 5);
    assert(result == 0);
    
    size_t out_size;
    unsigned char *out_data = read_test_file("test_output8.bin", &out_size);
    assert(out_data);
    assert(out_size == sizeof(expected)-1);
    assert(memcmp(out_data, expected, out_size) == 0);
    free(out_data);
    
    printf("PASSED\n");
}

// Тест 9: замена с увеличением размера
void test_increase_size() {
    printf("Test 9: Replacement increasing size... ");
    unsigned char input[] = "abcdef";
    unsigned char old_seq[] = "cde";
    unsigned char new_seq[] = "12345";
    unsigned char expected[] = "ab12345f";
    
    create_test_file("test_input9.bin", input, sizeof(input)-1);
    
    ByteSequence old_s = {old_seq, sizeof(old_seq)-1};
    ByteSequence new_s = {new_seq, sizeof(new_seq)-1};
    
    int result = replace_bytes_in_file("test_input9.bin", "test_output9.bin", 
                                       &old_s, &new_s, 4);
    assert(result == 0);
    
    size_t out_size;
    unsigned char *out_data = read_test_file("test_output9.bin", &out_size);
    assert(out_data);
    assert(out_size == sizeof(expected)-1);
    assert(memcmp(out_data, expected, out_size) == 0);
    free(out_data);
    
    printf("PASSED\n");
}

// Тест 10: перекрывающиеся вхождения
void test_overlapping_matches() {
    printf("Test 10: Overlapping matches... ");
    unsigned char input[] = "aaaaa";
    unsigned char old_seq[] = "aa";
    unsigned char new_seq[] = "b";
    unsigned char expected[] = "bab";
    
    create_test_file("test_input10.bin", input, sizeof(input)-1);
    
    ByteSequence old_s = {old_seq, sizeof(old_seq)-1};
    ByteSequence new_s = {new_seq, sizeof(new_seq)-1};
    
    int result = replace_bytes_in_file("test_input10.bin", "test_output10.bin", 
                                       &old_s, &new_s, 3);
    assert(result == 0);
    
    size_t out_size;
    unsigned char *out_data = read_test_file("test_output10.bin", &out_size);
    assert(out_data);
    assert(out_size == sizeof(expected)-1);
    assert(memcmp(out_data, expected, out_size) == 0);
    free(out_data);
    
    printf("PASSED\n");
}

// Функция очистки тестовых файлов
void cleanup() {
    remove("test_input1.bin");
    remove("test_output1.bin");
    remove("test_input2.bin");
    remove("test_output2.bin");
    remove("test_input3.bin");
    remove("test_output3.bin");
    remove("test_input4.bin");
    remove("test_output4.bin");
    remove("test_input5.bin");
    remove("test_output5.bin");
    remove("test_input6.bin");
    remove("test_output6.bin");
    remove("test_input7.bin");
    remove("test_output7.bin");
    remove("test_input8.bin");
    remove("test_output8.bin");
    remove("test_input9.bin");
    remove("test_output9.bin");
    remove("test_input10.bin");
    remove("test_output10.bin");
}

int main() {
    printf("Running byte replacer tests...\n");
    printf("================================\n");
    
    test_basic_replacement();
    test_replacement_at_start();
    test_replacement_at_end();
    test_multiple_replacements();
    test_partial_match_boundary();
    test_null_bytes();
    test_sequence_larger_than_block();
    test_empty_replacement();
    test_increase_size();
    test_overlapping_matches();
    
    printf("================================\n");
    printf("All tests PASSED!\n");
    
    cleanup();
    return 0;
}