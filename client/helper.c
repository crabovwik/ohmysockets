#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <syscall.h>
#include <linux/random.h>

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
