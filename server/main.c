#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <helper.h>
#include <mbedtls/md5.h>
#include <network.h>
#include <pthread.h>
#include <stdlib.h>

#include "main.h"

#define PACKETS_BUFFER_SIZE 16
#define READ_CHUNK_SIZE 4096
#define LISTEN_BACKLOG_SIZE 1024

static char *STATUS_PASS = "PASS";
static char *STATUS_FAIL = "FAIL";

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

static struct packet_with_validation *packets_buffer[PACKETS_BUFFER_SIZE];
static volatile int packets_buffer_elements_count = 0;

void *worker(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        while (packets_buffer_elements_count == 0) {
            pthread_cond_wait(&condition, &mutex);
        }

        struct packet_with_validation *packet;
        for (unsigned int i = 0; i < PACKETS_BUFFER_SIZE; i++) {
            packet = packets_buffer[i];
            if (packet == NULL) {
                continue;
            }

            char *status_message = get_validation_message_by_validation_status(packet->is_valid);
            printf("Processed: #%d #%lu %s\n", packet->packet->number, packet->packet->microtime, status_message);

            free(packet->packet);
            free(packet);
            packets_buffer[i] = NULL;
            packets_buffer_elements_count--;

            usleep(1000 * 15);
        }

        pthread_mutex_unlock(&mutex);
    }
}

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

    if (listen(socketfd, LISTEN_BACKLOG_SIZE) < 0) {
        helper_error_message("listen");
        return 1;
    }

    pthread_t pthread;
    pthread_create(&pthread, NULL, worker, NULL);

    int connectionfd;
    char buffer[NETWORK_PACKET_TOTAL_SIZE];
    bzero(buffer, NETWORK_PACKET_TOTAL_SIZE);
    // epoll?
    while (1) {
        if ((connectionfd = accept(socketfd, (struct sockaddr *) NULL, NULL)) < 0) {
            helper_error_message("accept");
            return 1;
        }


        while (read_not_less_than(connectionfd, buffer, NETWORK_PACKET_TOTAL_SIZE) == 0) {
            struct packet *network_packet = (struct packet *) malloc(sizeof(struct packet));
            network_decode_packet(buffer, network_packet);

            char md5_generated[NETWORK_PACKET_MD5_SIZE];
            if (mbedtls_md5_ret((const unsigned char *) &network_packet->data, NETWORK_PACKET_DATA_SIZE, (unsigned char *) md5_generated) != 0) {
                helper_error_message("mbedtls_md5_ret");
                return 1;
            }

            struct packet_with_validation *packet_with_validation = (struct packet_with_validation *) malloc(sizeof(struct packet_with_validation));
            packet_with_validation->packet = network_packet;
            packet_with_validation->is_valid = memcmp(&network_packet->md5, md5_generated, NETWORK_PACKET_MD5_SIZE) == 0;

            pthread_mutex_lock(&mutex);

            unsigned int index;
            if (packets_buffer_elements_count < PACKETS_BUFFER_SIZE) {
                index = packets_buffer_elements_count;
            } else if (packets_buffer_elements_count % PACKETS_BUFFER_SIZE == 0) {
                index = 0;
            } else {
                index = packets_buffer_elements_count % PACKETS_BUFFER_SIZE - 1;
            }

            packets_buffer[index] = packet_with_validation;
            packets_buffer_elements_count++;

            pthread_cond_signal(&condition);
            pthread_mutex_unlock(&mutex);

            char *status_message = get_validation_message_by_validation_status(packet_with_validation->is_valid);
            printf("Received: #%d #%lu %s\n", network_packet->number, network_packet->microtime, status_message);
        }
    }
}

char *get_validation_message_by_validation_status(int i) {
    return i ? STATUS_PASS : STATUS_FAIL;
}

unsigned int read_not_less_than(int fd, char *buffer, unsigned int size) {
    unsigned int total_read_bytes = 0;

    int read_bytes_count;
    unsigned int chunk_size;
    unsigned int size_left;

    do {
        size_left = size - total_read_bytes;
        chunk_size = size_left < READ_CHUNK_SIZE ? size_left : READ_CHUNK_SIZE;

        read_bytes_count = read(fd, buffer + total_read_bytes, chunk_size);
        if (read_bytes_count <= 0) {
            return 1;
        }

        total_read_bytes += read_bytes_count;
    } while ((size - total_read_bytes) != 0);

    return 0;
}
