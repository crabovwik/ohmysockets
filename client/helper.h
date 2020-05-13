#ifndef CLIENT_HELPER_H
#define CLIENT_HELPER_H

#define MD5_SIZE_BYTES 16
#define MD5_SIZE_HEX (MD5_SIZE_BYTES * 2)

void helper_bytes_to_hex(const unsigned char *const input, unsigned int length, unsigned char *const output);
unsigned int helper_md5(const unsigned char * const input, unsigned int length, const unsigned char output[MD5_SIZE_HEX + 1]);

#endif //CLIENT_HELPER_H
