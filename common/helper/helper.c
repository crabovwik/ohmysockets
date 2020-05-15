#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syscall.h>
#include <strings.h>
#include <endian.h>
#include <errno.h>
#include <sys/time.h>

#include "helper.h"

void helper_bytes_to_hex(const unsigned char *const input, unsigned int size, unsigned char *const output) {
    for (unsigned int input_index = 0, output_index = 0; input_index < size; input_index++, output_index += 2) {
        sprintf(((char *) output) + output_index, "%02x", input[input_index]);
    }
}

unsigned int
helper_md5(const unsigned char *const input, unsigned int size, const unsigned char output[MD5_SIZE_HEX + 1]) {
    unsigned char hash_bytes[MD5_SIZE_BYTES];
    if ((mbedtls_md5_ret(input, size, (unsigned char *) hash_bytes)) != 0) {
        return 1;
    }

    helper_bytes_to_hex((unsigned const char *const) hash_bytes, MD5_SIZE_BYTES, (unsigned char *const) output);

    return 0;
}

unsigned int helper_fill_with_int16(int16_t *const buffer, unsigned int size) {
    if (size % sizeof(int16_t) != 0) {
        return 1;
    }

    if (helper_get_random_bytes((unsigned char *const) buffer, size) != 0) {
        return 1;
    }

    return 0;
}

unsigned int helper_get_random_bytes(unsigned char *const buffer, unsigned int size) {
    if (syscall(SYS_getrandom, (void *) buffer, size, NULL/*GRND_NONBLOCK*/) != size) {
        return 1;
    }

    return 0;
}

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

void header_flip_int32(int32_t *ptr) {
    helper_flip_bytes((char *) ptr, sizeof(int32_t));
}

void header_flip_int32_array(int32_t *ptr, unsigned int length) {
    for (unsigned int i = 0; i < length; i++) {
        header_flip_int32(ptr + i);
    }
}

uint64_t helper_get_current_time_in_microseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return 1000000 * tv.tv_sec + tv.tv_usec;
}
