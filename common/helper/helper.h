#ifndef CLIENT_HELPER_H
#define CLIENT_HELPER_H

#include <inttypes.h>

#define PACKET_DATA_INT16_WORDS_COUNT 700
#define PACKET_DATA_SIZE (PACKET_DATA_INT16_WORDS_COUNT * sizeof(int16_t))
#define PACKET_NUMBER_SIZE sizeof(uint16_t)
#define PACKET_MICROTIME_SIZE sizeof(uint64_t)
#define PACKET_DATA_MAX_SIZE (sizeof(int16_t) * 1600)
#define PACKET_TOTAL_SIZE (PACKET_NUMBER_SIZE + PACKET_MICROTIME_SIZE + PACKET_DATA_MAX_SIZE + MD5_SIZE_BYTES)

#define MD5_SIZE_BYTES 16
#define MD5_SIZE_HEX (MD5_SIZE_BYTES * 2)

#define MAX_ASCII_CHARS_COUNT_IN_INT32_VALUE 6 // with minus

void helper_bytes_to_hex(const unsigned char *const input, unsigned int size, unsigned char *const output);

unsigned int
helper_md5(const unsigned char *const input, unsigned int size, const unsigned char output[MD5_SIZE_HEX + 1]);

unsigned int helper_fill_with_int16(int16_t *const buffer, unsigned int size);

void helper_initialize_random(const unsigned int *const seed);

unsigned int helper_get_random_bytes(unsigned char *const buffer, unsigned int size);

unsigned int helper_cycle_read_from_csv_file(FILE *fd, int16_t *const buffer, unsigned int size);

unsigned int helper_read_int16_from_csv_file(FILE *const fd, int16_t *const buffer);

unsigned int helper_get_system_endian();

void helper_flip_bytes(char *const buffer, unsigned int length);

uint64_t helper_get_current_time_in_microseconds();

void header_flip_int32(int32_t *ptr);

void header_flip_int32_array(int32_t *ptr, unsigned int length);

#endif //CLIENT_HELPER_H
