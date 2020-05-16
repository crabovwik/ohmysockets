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
#include <helper.h>
#include <mbedtls/md5.h>

#define BATCH_LOOPS 2
#define BATCH_SIZE 100

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: %s ip port\n", argv[0]);
        return 1;
    }

    FILE *csvfd = fopen("../client/data.csv", "r");
    if (csvfd == NULL) {
        helper_error_message("fopen");
        return 1;
    }

    int socketfd;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        helper_error_message("socket");
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;

    unsigned short port;
    if (helper_parse_port(argv[2], &port) != 0) {
        helper_error_message("port parsing");
        return 1;
    }

    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        helper_error_message("inet_pton");
        return 1;
    }

    if (connect(socketfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        helper_error_message("connect");
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
                helper_error_message("helper_cycle_read_from_csv_file");
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
                helper_error_message("mbedtls_md5_ret");
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
                helper_error_message("write");
                return 1;
            }

            printf("Sent: #%d #%lu\n", packet_number, original_microtime);

            // packet sleep
            usleep(1000 * 10);
        }

        if (i != (BATCH_LOOPS - 1)) {
            sleep(10);
        }
    }

    return 0;
}
