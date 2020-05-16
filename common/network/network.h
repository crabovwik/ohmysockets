#ifndef COMMON_NETWORK_H
#define COMMON_NETWORK_H

#define NETWORK_PACKET_NUMBER_SIZE sizeof(uint16_t)
#define NETWORK_PACKET_MICROTIME_SIZE sizeof(uint64_t)
#define NETWORK_PACKET_DATA_SIZE (NETWORK_PACKET_DATA_INT16_WORDS_COUNT * sizeof(int16_t))
#define NETWORK_PACKET_DATA_INT16_WORDS_COUNT 700
#define NETWORK_PACKET_MD5_SIZE 16

#define NETWORK_PACKET_TOTAL_SIZE (NETWORK_PACKET_NUMBER_SIZE + NETWORK_PACKET_MICROTIME_SIZE + NETWORK_PACKET_DATA_SIZE + NETWORK_PACKET_MD5_SIZE)

struct packet {
    uint16_t number;
    uint64_t microtime;
    int16_t data[NETWORK_PACKET_DATA_INT16_WORDS_COUNT];
    char md5[NETWORK_PACKET_MD5_SIZE];
};

void network_encode_packet(struct packet *packet, char *buffer);

void network_decode_packet(char *buffer, struct packet *packet);

#endif //COMMON_NETWORK_H
