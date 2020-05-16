#ifndef COMMON_HELPER_H
#define COMMON_HELPER_H

#include <stdio.h>
#include <inttypes.h>

#define MD5_SIZE_BYTES 16

#define MAX_ASCII_CHARS_COUNT_IN_INT32_VALUE 6

unsigned int helper_cycle_read_from_csv_file(FILE *fd, int16_t *const buffer, unsigned int size);

unsigned int helper_read_int16_from_csv_file(FILE *const fd, int16_t *const buffer);

unsigned int helper_get_system_endian();

void helper_flip_bytes(char *const buffer, unsigned int length);

uint64_t helper_get_current_time_in_microseconds();

unsigned short int helper_parse_port(char *data, unsigned short *result);

void helper_error_message(char *message);

#endif //COMMON_HELPER_H
