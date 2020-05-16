#include <stdlib.h>
#include <inttypes.h>
#include <helper.h>
#include "network.h"
#include <string.h>
#include <netinet/in.h>

void network_encode_packet(struct packet *packet, char *buffer) {
    // packet number
    uint16_t *packet_number_ptr = (uint16_t *) buffer;
    *packet_number_ptr = htons(packet->number);

    // microtime
    uint64_t *microtime_ptr = (uint64_t *) (((char *) packet_number_ptr) + NETWORK_PACKET_NUMBER_SIZE);
    *microtime_ptr = packet->microtime;
    if (helper_get_system_endian() != BIG_ENDIAN) {
        helper_flip_bytes((char *) microtime_ptr, sizeof(uint64_t));
    }

    // data
    int16_t *data_ptr = (int16_t *) (((char *) microtime_ptr) + NETWORK_PACKET_MICROTIME_SIZE);
    memcpy(data_ptr, packet->data, NETWORK_PACKET_DATA_SIZE);
    if (helper_get_system_endian() != BIG_ENDIAN) {
        for (unsigned int i = 0; i < NETWORK_PACKET_DATA_SIZE; i++) {
            data_ptr[i] = htons(data_ptr[i]);
        }
    }

    // md5
    unsigned char *md5_ptr = (unsigned char *) (((char *) data_ptr) + NETWORK_PACKET_DATA_SIZE);
    memcpy(md5_ptr, packet->md5, MD5_SIZE_BYTES);
    if (helper_get_system_endian() != BIG_ENDIAN) {
        helper_flip_bytes((char *) md5_ptr, MD5_SIZE_BYTES);
    }
}

void network_decode_packet(char *buffer, struct packet *packet) {
    // packet number
    uint16_t *packet_number_ptr = (uint16_t *) buffer;
    packet->number = htons(*packet_number_ptr);

    // microtime
    uint64_t *microtime_ptr = (uint64_t *) (((char *) packet_number_ptr) + NETWORK_PACKET_NUMBER_SIZE);
    packet->microtime = *microtime_ptr;

    if (helper_get_system_endian() != BIG_ENDIAN) {
        helper_flip_bytes((char *) &packet->microtime, NETWORK_PACKET_MICROTIME_SIZE);
    }

    // data
    int16_t *data_ptr = (int16_t *) (((char *) microtime_ptr) + NETWORK_PACKET_MICROTIME_SIZE);
    memcpy(&packet->data, data_ptr, NETWORK_PACKET_DATA_SIZE);

    if (helper_get_system_endian() != BIG_ENDIAN) {
        for (unsigned int i = 0; i < NETWORK_PACKET_DATA_INT16_WORDS_COUNT; i++) {
            packet->data[i] = ntohs(packet->data[i]);
        }
    }

    // md5
    unsigned char *md5_ptr = (unsigned char *) (((char *) data_ptr) + NETWORK_PACKET_DATA_SIZE);
    memcpy(&packet->md5, md5_ptr, MD5_SIZE_BYTES);

    if (helper_get_system_endian() != BIG_ENDIAN) {
        helper_flip_bytes((char *) &packet->md5, MD5_SIZE_BYTES);
    }
}
