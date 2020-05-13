#include <stdio.h>
#include <string.h>

#include "lib/mbedtls-2.16.6/include/mbedtls/md5.h"

#define MD5_SIZE_BYTES 16
#define MD5_SIZE_HEX (MD5_SIZE_BYTES * 2)

void bytes_to_hex(unsigned const char *const input, unsigned int length, unsigned char *const output);

int main() {
    char *buffer = "123";

    unsigned char hash_bytes[MD5_SIZE_BYTES];
    if (mbedtls_md5_ret((const unsigned char *) buffer, strlen(buffer), hash_bytes) != 0) {
        printf("[err] mbedtls_md5_ret\n");

        return 1;
    }

    unsigned char hash_string[MD5_SIZE_HEX + 1];
    bytes_to_hex((unsigned const char *const) hash_bytes, MD5_SIZE_BYTES, (unsigned char *const) hash_string);

    printf("hash: %s\n", hash_string);

    return 0;
}

void bytes_to_hex(unsigned const char *const input, unsigned int length, unsigned char *const output) {
    for (unsigned int input_index = 0, output_index = 0; input_index < length; input_index++, output_index += 2) {
        sprintf(((char *) output) + output_index, "%02x", input[input_index]);
    }
}
