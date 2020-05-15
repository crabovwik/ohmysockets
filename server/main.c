#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../common/helper/helper.h"

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

    char *endptr;
    short int port = (short int) strtoul(argv[1], &endptr, 10);
    switch (errno) {
        case ERANGE:
        case EINVAL:
            printf("[error] port parsing\n");
            return 1;
    }

    server_address.sin_port = ntohs(port);

    if (bind(socketfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        printf("[error] bind\n");
        return 1;
    }

    if (listen(socketfd, 1024) < 0) {
        printf("[error] listen\n");
        return 1;
    }

    int connectionfd;
    int read_bytes;
    char buffer[3226];
    bzero(buffer, 3226);
    // epoll?
    while (1) {
        if ((connectionfd = accept(socketfd, (struct sockaddr *) NULL, NULL)) < 0) {
            printf("[error] accept\n");
            return 1;
        }

        while ((read_bytes = read(connectionfd, buffer, 3226)) > 0) {
            printf("received size: %d\n", read_bytes);

            uint16_t *packet_number_ptr = (uint16_t *) buffer;
            uint64_t *microtime_ptr = (uint64_t *) (((char *) packet_number_ptr) + sizeof(uint16_t));
            int16_t *data_ptr = (int16_t *) (((char *) microtime_ptr) + sizeof(uint64_t));
            unsigned char *md5_ptr = ((unsigned char *) data_ptr) + (sizeof(int16_t) * 1600);

            // packet number
            uint16_t packet_number = ntohs(*packet_number_ptr);

            // microtime
            uint64_t microtime = 0;
            memcpy(&microtime, microtime_ptr, sizeof(uint64_t));
            if (helper_get_system_endian() != BIG_ENDIAN) {
                helper_flip_bytes((char *) &microtime, sizeof(uint64_t));
            }

            // data
            int16_t data[1600];
            memcpy(data, data_ptr, sizeof(int16_t) * 1600);
            for (unsigned int i = 0; i < 700; i++) {
                data[i] = ntohs(data[i]);
            }

            // md5
            unsigned char md5[16];
            memcpy(md5, md5_ptr, 16);
            if (helper_get_system_endian() != BIG_ENDIAN) {
                helper_flip_bytes((char *) md5, 16);
            }

            char md5_generated[16];
            if (mbedtls_md5_ret((const unsigned char *) data, sizeof(int16_t) * 700,
                                (unsigned char *) md5_generated) != 0) {
                printf("[error] mbedtls_md5_ret\n");
                return 1;
            }

            char *status = memcmp(md5, md5_generated, 16) == 0 ? STATUS_PASS : STATUS_FAIL;

            printf("Received: #%d #%lu %s\n", packet_number, microtime, status);
        }
    }
}
