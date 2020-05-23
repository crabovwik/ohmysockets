#include <stdlib.h>
#include <helper.h>

#include "main.h"
#include "circular_buffer.h"

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
        helper_error_message("pthread_mutex_lock");
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
        helper_error_message("pthread_mutex_unlock");
        return NULL;
    }

    return packet;
}

int cb_push(struct circular_buffer *cb, struct packet_with_validation *const packet) {
    if (pthread_mutex_lock(cb->mutex) != 0) {
        helper_error_message("pthread_mutex_lock");
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
        helper_error_message("pthread_mutex_unlock");
        return -1;
    }

    return 0;
}

unsigned int cb_current_length(struct circular_buffer *cb) {
    if (pthread_mutex_lock(cb->mutex) != 0) {
        helper_error_message("pthread_mutex_lock");
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
        helper_error_message("pthread_mutex_unlock");
        return -1;
    }

    return return_length;
}