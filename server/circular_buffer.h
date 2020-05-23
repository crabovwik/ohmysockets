#ifndef SERVER_CIRCULAR_BUFFER_H
#define SERVER_CIRCULAR_BUFFER_H

#include <pthread.h>

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

#endif //SERVER_CIRCULAR_BUFFER_H
