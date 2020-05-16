#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <helper.h>
#include <mbedtls/md5.h>

static char *STATUS_PASS = "pass";
static char *STATUS_FAIL = "fail";

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s port\n", argv[0]);
        return 1;
    }

    int socketfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    unsigned short port;
    if (helper_parse_port(argv[1], &port) != 0) {
        helper_error_message("port parsing");
        return 1;
    }

    server_address.sin_port = htons(port);

    if (bind(socketfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        helper_error_message("bind");
        return 1;
    }

    if (listen(socketfd, 1024) < 0) {
        helper_error_message("listen");
        return 1;
    }

    int connectionfd;
    int read_bytes;
    char buffer[PACKET_TOTAL_SIZE];
    bzero(buffer, PACKET_TOTAL_SIZE);
    // epoll?
    while (1) {
        if ((connectionfd = accept(socketfd, (struct sockaddr *) NULL, NULL)) < 0) {
            helper_error_message("accept");
            return 1;
        }

        while ((read_bytes = read(connectionfd, buffer, PACKET_TOTAL_SIZE)) > 0) {
            uint16_t *packet_number_ptr = (uint16_t *) buffer;
            uint64_t *microtime_ptr = (uint64_t *) (((char *) packet_number_ptr) + PACKET_NUMBER_SIZE);
            int16_t *data_ptr = (int16_t *) (((char *) microtime_ptr) + PACKET_MICROTIME_SIZE);
            unsigned char *md5_ptr = ((unsigned char *) data_ptr) + PACKET_DATA_MAX_SIZE;

            // packet number
            uint16_t packet_number = ntohs(*packet_number_ptr);

            // microtime
            uint64_t microtime = 0;
            memcpy(&microtime, microtime_ptr, sizeof(uint64_t));
            if (helper_get_system_endian() != BIG_ENDIAN) {
                helper_flip_bytes((char *) &microtime, sizeof(uint64_t));
            }

            // data
            int16_t data[PACKET_DATA_INT16_WORDS_COUNT];
            memcpy(data, data_ptr, PACKET_DATA_SIZE);
            for (unsigned int i = 0; i < PACKET_DATA_INT16_WORDS_COUNT; i++) {
                data[i] = ntohs(data[i]);
            }

            // md5
            unsigned char md5[MD5_SIZE_BYTES];
            memcpy(md5, md5_ptr, MD5_SIZE_BYTES);
            if (helper_get_system_endian() != BIG_ENDIAN) {
                helper_flip_bytes((char *) md5, MD5_SIZE_BYTES);
            }

            char md5_generated[MD5_SIZE_BYTES];
            if (mbedtls_md5_ret((const unsigned char *) data, PACKET_DATA_SIZE,
                                (unsigned char *) md5_generated) != 0) {
                helper_error_message("mbedtls_md5_ret");
                return 1;
            }

            char *status = memcmp(md5, md5_generated, MD5_SIZE_BYTES) == 0 ? STATUS_PASS : STATUS_FAIL;

            printf("Received: #%d #%lu %s\n", packet_number, microtime, status);
        }
    }
}
