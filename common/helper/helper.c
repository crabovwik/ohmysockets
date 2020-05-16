#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <endian.h>
#include <errno.h>
#include <sys/time.h>

#include "helper.h"

unsigned int helper_cycle_read_from_csv_file(FILE *fd, int16_t *const buffer, unsigned int size) {
    if (size % sizeof(int16_t) != 0) {
        return 1;
    }

    unsigned int elements_count = size / sizeof(int16_t);
    for (unsigned int i = 0; i < elements_count; i++) {
        if (helper_read_int16_from_csv_file(fd, buffer + i) != 0) {
            printf("[err] helper_read_int16_from_csv_file\n");
            return 1;
        }
    }

    return 0;
}

unsigned int helper_read_int16_from_csv_file(FILE *const fd, int16_t *const buffer) {
    unsigned char ascii_int16[MAX_ASCII_CHARS_COUNT_IN_INT32_VALUE];
    bzero(ascii_int16, MAX_ASCII_CHARS_COUNT_IN_INT32_VALUE);

    int i = 0;
    int c;
    while (1) {
        if ((c = fgetc(fd)) == EOF) {
            fseek(fd, 0, SEEK_SET);
        }

        if ((c >= '0' && c <= '9') || (c == '-' && i == 0)) {
            ascii_int16[i++] = (unsigned char) c;
            continue;
        }

        if (i != 0) {
            break;
        }
    }

    char *end;
    int16_t value = (int16_t) strtoul((const char *) ascii_int16, &end, 10);
    switch (errno) {
        case ERANGE:
        case EINVAL:
            return 1;
    }

    *buffer = value;

    return 0;
}

unsigned int helper_get_system_endian() {
    int i = 1;
    char *p = (char *) &i;

    return *p == 1 ? LITTLE_ENDIAN : BIG_ENDIAN;
}

void helper_flip_bytes(char *const buffer, unsigned int length) {
    char tmp_byte;
    for (unsigned int left_index = 0, right_index = length - 1; left_index < length; left_index++, right_index--) {
        if (left_index == right_index || left_index > right_index) {
            break;
        }

        tmp_byte = buffer[left_index];
        buffer[left_index] = buffer[right_index];
        buffer[right_index] = tmp_byte;
    }
}

uint64_t helper_get_current_time_in_microseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return 1000000 * tv.tv_sec + tv.tv_usec;
}

unsigned short int helper_parse_port(char *data, unsigned short *result) {
    char *endptr;
    *result = (unsigned short int) strtoul(data, &endptr, 10);
    switch (errno) {
        case ERANGE:
        case EINVAL:
            return 1;
    }

    return 0;
}

void helper_error_message(char *message) {
    printf("[error] %s\n", message);
}
