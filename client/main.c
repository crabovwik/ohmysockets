#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>

#include "../common/helper/helper.h"

#define PACKET_DATA_INT16_WORDS_COUNT 700
#define PACKET_DATA_SIZE (PACKET_DATA_INT16_WORDS_COUNT * sizeof(int16_t))
#define PACKET_NUMBER_SIZE sizeof(uint16_t)
#define PACKET_MICROTIME_SIZE sizeof(uint64_t)
#define PACKET_DATA_MAX_SIZE (sizeof(int16_t) * 1600)
#define PACKET_TOTAL_SIZE (PACKET_NUMBER_SIZE + PACKET_MICROTIME_SIZE + PACKET_DATA_MAX_SIZE + MD5_SIZE_BYTES)
#define BATCH_LOOPS 1
#define BATCH_SIZE 50

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: %s ip port\n", argv[0]);
        return 1;
    }

    FILE *csvfd = fopen("../client/data.csv", "r");
    if (csvfd == NULL) {
        printf("[error] fopen\n");
        return 1;
    }

    int socketfd;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[error] socket\n");
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;

    char *endptr;
    short int port = (short int) strtoul(argv[2], &endptr, 10);
    switch (errno) {
        case ERANGE:
        case EINVAL:
            printf("[error] port parsing\n");
            return 1;
    }

    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        printf("[error] inet_pton\n");
        return 1;
    }

    if (connect(socketfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        printf("[error] connect\n");
        return 1;
    }

    uint16_t packet_number;
    int16_t data[PACKET_DATA_INT16_WORDS_COUNT];
    int16_t origin_data[PACKET_DATA_INT16_WORDS_COUNT];
    for (unsigned int i = 0; i < BATCH_LOOPS; i++) {
        for (unsigned int j = 0; j < BATCH_SIZE; j++) {
            char *result_packet_buffer = malloc(PACKET_TOTAL_SIZE);
            bzero(result_packet_buffer, PACKET_TOTAL_SIZE);

            // packet number
            uint16_t *packet_number_ptr = (uint16_t *) result_packet_buffer;

            packet_number = (uint16_t) ((i * BATCH_SIZE) + (j + 1));
            *packet_number_ptr = htons(packet_number);

            // microtime
            uint64_t *microtime_ptr = (uint64_t *) (result_packet_buffer + PACKET_NUMBER_SIZE);
            uint64_t original_microtime = (uint64_t) helper_get_current_time_in_microseconds();
            uint64_t network_microtime = original_microtime;
            if (helper_get_system_endian() != BIG_ENDIAN) {
                helper_flip_bytes((char *) &network_microtime, sizeof(uint64_t));
            }

            *microtime_ptr = network_microtime;

            // data
            if (helper_cycle_read_from_csv_file(csvfd, data, PACKET_DATA_SIZE) != 0) {
                printf("[error] helper_cycle_read_from_csv_file\n");
                return 1;
            }

            for (unsigned int z = 0; z < PACKET_DATA_INT16_WORDS_COUNT; z++) {
                origin_data[z] = data[z];
                data[z] = htons(data[z]);
            }

            memcpy(result_packet_buffer + PACKET_NUMBER_SIZE + PACKET_MICROTIME_SIZE, data, PACKET_DATA_SIZE);

            // md5
            unsigned char hash_bytes[MD5_SIZE_BYTES];
            bzero(hash_bytes, MD5_SIZE_BYTES);
            if ((mbedtls_md5_ret((const unsigned char *) origin_data, PACKET_DATA_SIZE,
                                 (unsigned char *) hash_bytes)) != 0) {
                printf("[error] mbedtls_md5_ret\n");
                return 1;
            }

            if (helper_get_system_endian() != BIG_ENDIAN) {
                helper_flip_bytes((char *) hash_bytes, MD5_SIZE_BYTES);
            }

            memcpy(
                    result_packet_buffer + PACKET_NUMBER_SIZE + PACKET_MICROTIME_SIZE + PACKET_DATA_MAX_SIZE,
                    hash_bytes,
                    MD5_SIZE_BYTES
            );

            // send
            if (write(socketfd, result_packet_buffer, PACKET_TOTAL_SIZE) != PACKET_TOTAL_SIZE) {
                printf("[error] write\n");
                return 1;
            }

            printf("Sent: #%d #%lu\n", packet_number, original_microtime);

            for (unsigned int k = 0; k < 3; k++) {
                printf(" %hu", origin_data[k]);
            }

            printf("\n");

            // packet sleep
            usleep(1000 * 10);
        }

        if (i != (BATCH_LOOPS - 1)) {
            sleep(10);
        }
    }

    return 0;
}
