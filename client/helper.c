#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <syscall.h>
#include <linux/random.h>
#include <strings.h>

#include "helper.h"
#include "lib/mbedtls-2.16.6/include/mbedtls/md5.h"

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

//        printf("c: %c\n", c);

        if ((c >= '0' && c <= '9') || (c == '-' && i == 0)) {
            ascii_int16[i++] = (unsigned char) c;
            continue;
        }

        if (i != 0) {
            break;
        }
    }

//    char *end;
//
//    printf("%s\n", ascii_int16);
//    printf("%hd\n", atoi(ascii_int16));
//    printf("%hd\n", strtoul((const char *) ascii_int16, &end, 10));
//
//    exit(1);

//    strtoul()
    char *end;
    *buffer = (int16_t) strtoul((const char *) ascii_int16, &end, 10);
//    *buffer = (int16_t) atoi(ascii_int16); // NOLINT (ert-err34-c)

    return 0;
}
