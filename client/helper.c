#include <stdio.h>

#include "helper.h"
#include "lib/mbedtls-2.16.6/include/mbedtls/md5.h"

void helper_bytes_to_hex(const unsigned char *const input, unsigned int length, unsigned char *const output) {
    for (unsigned int input_index = 0, output_index = 0; input_index < length; input_index++, output_index += 2) {
        sprintf(((char *) output) + output_index, "%02x", input[input_index]);
    }
}

unsigned int helper_md5(const unsigned char * const input, unsigned int length, const unsigned char output[MD5_SIZE_HEX + 1]) {
    unsigned char hash_bytes[MD5_SIZE_BYTES];
    if ((mbedtls_md5_ret(input, length, (unsigned char *) hash_bytes)) != 0) {
        return 1;
    }

    helper_bytes_to_hex((unsigned const char *const) hash_bytes, MD5_SIZE_BYTES, (unsigned char *const) output);

    return 0;
}
