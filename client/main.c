#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <inttypes.h>
#include <helper.h>
#include <mbedtls/md5.h>
#include <network.h>

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

    char result_packet_buffer[NETWORK_PACKET_TOTAL_SIZE];
    struct packet network_packet;
    for (unsigned int i = 0; i < BATCH_LOOPS; i++) {
        for (unsigned int j = 0; j < BATCH_SIZE; j++) {
            network_packet.number = (uint16_t) ((i * BATCH_SIZE) + (j + 1));
            network_packet.microtime = helper_get_current_time_in_microseconds();

            if (helper_cycle_read_from_csv_file(csvfd, (int16_t *) &network_packet.data, NETWORK_PACKET_DATA_SIZE) != 0) {
                helper_error_message("helper_cycle_read_from_csv_file");
                return 1;
            }

            if (mbedtls_md5_ret((unsigned char *) &network_packet.data, NETWORK_PACKET_DATA_SIZE, (unsigned char *) &network_packet.md5) != 0) {
                helper_error_message("mbedtls_md5_ret");
                return 1;
            }

            network_encode_packet(&network_packet, result_packet_buffer);

            if (write(socketfd, result_packet_buffer, NETWORK_PACKET_TOTAL_SIZE) != NETWORK_PACKET_TOTAL_SIZE) {
                helper_error_message("write");
                return 1;
            }

            printf("Sent: #%d #%lu\n", network_packet.number, network_packet.microtime);

            // packet sleep
            usleep(1000 * 10);
        }

        if (i != (BATCH_LOOPS - 1)) {
            sleep(10);
        }
    }

    return 0;
}
