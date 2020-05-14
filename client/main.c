#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"

#define PACKET_WORDS_COUNT 700
#define PACKET_WORDS_SIZE (PACKET_WORDS_COUNT * sizeof(int16_t))

int main() {
    char *buffer = "123";
    unsigned char buffer_hash[MD5_SIZE_HEX + 1];

    if ((helper_md5((unsigned char *) buffer, strlen(buffer), buffer_hash)) != 0) {
        printf("[error] helper_md5\n");
        return 1;
    }

    printf("[ok] hash: %s\n", buffer_hash);

    FILE *csvfd = fopen("../client/data.csv", "r");
    if (csvfd == NULL) {
        printf("[error] fopen\n");
        return 1;
    }

    int16_t words[PACKET_WORDS_COUNT];
    bzero(words, PACKET_WORDS_SIZE);
    if (helper_cycle_read_from_csv_file(csvfd, words, PACKET_WORDS_SIZE) != 0) {
        printf("[error] helper_cycle_read_from_csv_file\n");
        return 1;
    }

    for (unsigned int i = 0; i < PACKET_WORDS_COUNT; i++) {
        printf("%d ~~> %hd\n", i + 1, words[i]);
    }

//    int16_t words[PACKET_WORDS_COUNT];
//    bzero(words, PACKET_WORDS_SIZE);
//    if (helper_fill_with_int16(words, PACKET_WORDS_SIZE) != 0) {
//        printf("[err] helper_fill_with_int16\n");
//        return 1;
//    }
//
//    for (unsigned int i = 0; i < PACKET_WORDS_COUNT; i++) {
//        printf("%d ~~~> %hd\n", i + 1, words[i]);
//    }

    return 0;
}
