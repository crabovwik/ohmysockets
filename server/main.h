#ifndef SERVER_MAIN_H
#define SERVER_MAIN_H

#include <network.h>

struct packet_with_validation {
    struct packet *packet;
    int is_valid;
};

struct circular_buffer {
    unsigned int length;
    pthread_mutex_t *mutex;

    unsigned int is_dirty;
    struct packet_with_validation **data;

    unsigned int push_index;
    unsigned int pull_index;
};

struct circular_buffer *cb_create(unsigned int length);

struct packet_with_validation *cb_pull(struct circular_buffer *cb);

int cb_push(struct circular_buffer *cb, struct packet_with_validation *const packet);

unsigned int cb_current_length(struct circular_buffer *cb);

char *get_validation_message_by_validation_status(int i);

int read_not_less_than(int fd, char *buffer, unsigned int size);

#endif //SERVER_MAIN_H

#pragma clang diagnostic pop