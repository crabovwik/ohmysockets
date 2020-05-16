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
#include <network.h>

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
    char buffer[NETWORK_PACKET_TOTAL_SIZE];
    bzero(buffer, NETWORK_PACKET_TOTAL_SIZE);
    // epoll?
    while (1) {
        if ((connectionfd = accept(socketfd, (struct sockaddr *) NULL, NULL)) < 0) {
            helper_error_message("accept");
            return 1;
        }

        struct packet network_packet;
        while ((read_bytes = read(connectionfd, buffer, NETWORK_PACKET_TOTAL_SIZE)) > 0) {
            if (read_bytes != NETWORK_PACKET_TOTAL_SIZE) {
                continue;
            }

            network_decode_packet(buffer, &network_packet);

            char md5_generated[MD5_SIZE_BYTES];
            if (mbedtls_md5_ret((const unsigned char *) &network_packet.data, NETWORK_PACKET_DATA_SIZE, (unsigned char *) md5_generated) != 0) {
                helper_error_message("mbedtls_md5_ret");
                return 1;
            }

            char *status = memcmp(&network_packet.md5, md5_generated, MD5_SIZE_BYTES) == 0 ? STATUS_PASS : STATUS_FAIL;

            printf("Received: #%d #%lu %s\n", network_packet.number, network_packet.microtime, status);
        }
    }
}
