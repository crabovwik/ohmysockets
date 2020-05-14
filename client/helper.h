#ifndef CLIENT_HELPER_H
#define CLIENT_HELPER_H

#define MD5_SIZE_BYTES 16
#define MD5_SIZE_HEX (MD5_SIZE_BYTES * 2)

void helper_bytes_to_hex(const unsigned char *const input, unsigned int size, unsigned char *const output);

unsigned int
helper_md5(const unsigned char *const input, unsigned int size, const unsigned char output[MD5_SIZE_HEX + 1]);

unsigned int helper_fill_with_int16(int16_t *const buffer, unsigned int size);

void helper_initialize_random(const unsigned int *const seed);

unsigned int helper_get_random_bytes(unsigned char *const buffer, unsigned int size);

#endif //CLIENT_HELPER_H
