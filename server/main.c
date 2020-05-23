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

struct circular_buffer *cb;

void *worker(void *arg) {
    while (1) {
        if (pthread_mutex_lock(&mutex) != 0) {
            helper_error_message("pthread_mutex_lock");
            break;
        }

        while (cb_current_length(cb) <= 0) {
            pthread_cond_wait(&condition, &mutex);
        }

        struct packet_with_validation *packet;
        while ((packet = cb_pull(cb)) != NULL) {
            char *status_message = get_validation_message_by_validation_status(packet->is_valid);
            printf("Processed: #%d #%lu %s\n", packet->packet->number, packet->packet->microtime, status_message);

            free(packet->packet);
            free(packet);

            usleep(1000 * 15);
        }

        if (pthread_mutex_unlock(&mutex) != 0) {
            helper_error_message("pthread_mutex_unlock");
            break;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s port\n", argv[0]);
        return 1;
    }


    if ((cb = cb_create(PACKETS_BUFFER_SIZE)) == NULL) {
        helper_error_message("cb_create");
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

    fd_set active_fd_set, read_fd_set;
    FD_ZERO(&active_fd_set);
    FD_SET(socketfd, &active_fd_set);

    pthread_t pthread;
    pthread_create(&pthread, NULL, worker, NULL);

    int i;
    int connectionfd;
    int read_status;
    char buffer[NETWORK_PACKET_TOTAL_SIZE];
    bzero(buffer, NETWORK_PACKET_TOTAL_SIZE);

    int maxfd = socketfd;

    while (1) {
        read_fd_set = active_fd_set;
        if (select(maxfd + 1, &read_fd_set, NULL, NULL, NULL) < 0) {
            helper_error_message("select");
            return 1;
        }

        for (i = 0; i <= maxfd; i++) {
            if (!FD_ISSET(i, &read_fd_set)) {
                continue;
            }

            if (i == socketfd) {
                if ((connectionfd = accept(i, (struct sockaddr *) NULL, NULL)) < 0) {
                    helper_error_message("accept");
                    return 1;
                }

                if (connectionfd >= FD_SETSIZE) {
                    helper_error_message("connectionfd greater than FD_SETSIZE");
                    close(connectionfd);
                    continue;
                }

                if (connectionfd > maxfd) {
                    maxfd = connectionfd;
                }

                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 50000; // 0.05 sec

                setsockopt(connectionfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

                FD_SET(connectionfd, &active_fd_set);
                continue;
            }

            read_status = read_not_less_than(i, buffer, NETWORK_PACKET_TOTAL_SIZE);
            if (read_status < 0) {
                close(i);
                FD_CLR(i, &active_fd_set);
                continue;
            }

            struct packet *network_packet = (struct packet *) malloc(sizeof(struct packet));
            network_decode_packet(buffer, network_packet);

            char md5_generated[NETWORK_PACKET_MD5_SIZE];
            if (mbedtls_md5_ret((const unsigned char *) &network_packet->data, NETWORK_PACKET_DATA_SIZE, (unsigned char *) md5_generated) !=
                0) {
                helper_error_message("mbedtls_md5_ret");
                return 1;
            }

            struct packet_with_validation *packet_with_validation = (struct packet_with_validation *) malloc(
                    sizeof(struct packet_with_validation));
            packet_with_validation->packet = network_packet;
            packet_with_validation->is_valid = memcmp(&network_packet->md5, md5_generated, NETWORK_PACKET_MD5_SIZE) == 0;

            pthread_mutex_lock(&mutex);

            if (cb_push(cb, packet_with_validation) != 0) {
                helper_error_message("cb_push");
                return 1;
            }

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

int read_not_less_than(int fd, char *buffer, unsigned int size) {
    unsigned int total_read_bytes = 0;

    int read_bytes_count;
    unsigned int chunk_size;
    unsigned int size_left;

    do {
        size_left = size - total_read_bytes;
        chunk_size = size_left < READ_CHUNK_SIZE ? size_left : READ_CHUNK_SIZE;

        read_bytes_count = read(fd, buffer + total_read_bytes, chunk_size);
        if (read_bytes_count == 0) {
            return EOF;
        }

        if (read_bytes_count < 0) {
            return read_bytes_count;
        }

        total_read_bytes += read_bytes_count;
    } while ((size - total_read_bytes) != 0);

    return 0;
}


struct circular_buffer *cb_create(unsigned int length) {
    if (length == 0) {
        return NULL;
    }

    struct circular_buffer *cb = calloc(1, sizeof(struct circular_buffer));

    cb->length = length;

    cb->data = (struct packet_with_validation **) malloc(sizeof(struct packet_with_validation) * length);

    cb->mutex = calloc(1, sizeof(pthread_mutex_t));
    if (pthread_mutex_init(cb->mutex, NULL) != 0) {
        return NULL;
    }

    return cb;
}

struct packet_with_validation *cb_pull(struct circular_buffer *cb) {
    if (pthread_mutex_lock(cb->mutex) != 0) {
        helper_error_message("oh fush");
        return NULL;
    }

    struct packet_with_validation *packet;

    if (!cb->is_dirty && cb->push_index == cb->pull_index) {
        packet = NULL;
    } else {
        packet = cb->data[cb->pull_index];

        cb->data[cb->pull_index] = NULL;
        cb->pull_index++;

        if (cb->pull_index == cb->length) {
            cb->pull_index = 0;
        }

        // all data has been read
        if (cb->push_index == cb->pull_index) {
            cb->is_dirty = 0;
        }
    }

    if (pthread_mutex_unlock(cb->mutex) != 0) {
        // todo: return pulled data again to the array
        helper_error_message("oh fuck");
        return NULL;
    }

    return packet;
}

int cb_push(struct circular_buffer *cb, struct packet_with_validation *const packet) {
    if (pthread_mutex_lock(cb->mutex) != 0) {
        helper_error_message("oh fush");
        return -1;
    }

    // overflow
    if (cb->is_dirty && cb->push_index == cb->pull_index) {
        cb->pull_index++;

        if (cb->pull_index == cb->length) {
            cb->pull_index = 0;
        }
    }

    // freeing
    if (cb->data[cb->push_index] != NULL) {
        free(cb->data[cb->push_index]->packet);
        free(cb->data[cb->push_index]);
    }

    cb->data[cb->push_index++] = packet;
    if (cb->push_index == cb->length) {
        cb->push_index = 0;
    }

    if (!cb->is_dirty) {
        cb->is_dirty = 1;
    }

    if (pthread_mutex_unlock(cb->mutex) != 0) {
        // todo: return pulled data again to the array
        helper_error_message("oh fush");
        return -1;
    }

    return 0;
}

unsigned int cb_current_length(struct circular_buffer *cb) {
    if (pthread_mutex_lock(cb->mutex) != 0) {
        helper_error_message("oh fuck");
        return -1;
    }

    unsigned int return_length;
    if (cb->push_index == cb->pull_index) {
        if (cb->is_dirty) {
            return_length = 1;
        } else {
            return_length = 0;
        }
    } else if (cb->push_index > cb->pull_index) {
        return_length = cb->push_index - cb->pull_index;
    } else {
        return_length = cb->length - (cb->pull_index - cb->push_index);
    }

    if (pthread_mutex_unlock(cb->mutex) != 0) {
        helper_error_message("oh fush");
        return -1;
    }

    return return_length;
}
