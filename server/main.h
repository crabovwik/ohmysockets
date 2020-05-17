#ifndef SERVER_MAIN_H
#define SERVER_MAIN_H

#include <network.h>

struct packet_with_validation {
    struct packet *packet;
    int is_valid;
};

char *get_validation_message_by_validation_status(int i);

unsigned int read_not_less_than(int fd, char *buffer, unsigned int size);

#endif //SERVER_MAIN_H
